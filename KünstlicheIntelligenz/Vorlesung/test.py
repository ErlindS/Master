"""
RISE demo: Randomized Input Sampling for Explanation (Petsiuk, Das & Saenko, 2018)

Explains the top predictions of a pretrained ImageNet classifier (ResNet50)
on any image, treating the model as a pure black box: only inputs and outputs.

Usage:
    python rise_demo.py highland_cattle.png
    python rise_demo.py highland_cattle.png --masks 8000 --topk 3 --out rise_result.png

Requirements:
    pip install torch torchvision matplotlib pillow
"""
import argparse

import matplotlib.pyplot as plt
import numpy as np
import torch
import torch.nn.functional as F
from PIL import Image
from torchvision import models

INPUT_SIZE = 224  # H = W = 224, as in the paper


def load_model(device):
    weights = models.ResNet50_Weights.IMAGENET1K_V2
    model = models.resnet50(weights=weights).eval().to(device)
    categories = weights.meta["categories"]
    return model, categories


def load_image(path):
    """Returns the original PIL image and a [1, 3, 224, 224] tensor with values in [0, 1]."""
    img = Image.open(path).convert("RGB")
    resized = img.resize((INPUT_SIZE, INPUT_SIZE), Image.BILINEAR)
    x = torch.from_numpy(np.array(resized)).float().permute(2, 0, 1) / 255.0
    return img, x.unsqueeze(0)


def normalize(x):
    """ImageNet normalization, applied AFTER masking so masked pixels are truly black."""
    mean = torch.tensor([0.485, 0.456, 0.406], device=x.device).view(1, 3, 1, 1)
    std = torch.tensor([0.229, 0.224, 0.225], device=x.device).view(1, 3, 1, 1)
    return (x - mean) / std


def generate_masks(n, generator, s=7, p=0.5, size=INPUT_SIZE):
    """
    Mask generation from Section 3.2 of the paper:
      1. Sample n binary grids of size s x s, each cell = 1 with probability p.
      2. Upsample bilinearly to (s+1) * cell, where cell = ceil(size / s).
      3. Crop a size x size window with a random offset in [0, cell).
    Result: smooth masks with values in [0, 1], shape [n, 1, size, size].
    """
    cell = int(np.ceil(size / s))
    up_size = (s + 1) * cell

    grid = (torch.rand(n, 1, s, s, generator=generator) < p).float()
    upsampled = F.interpolate(grid, size=(up_size, up_size), mode="bilinear", align_corners=False)

    masks = torch.empty(n, 1, size, size)
    shifts = torch.randint(0, cell, (n, 2), generator=generator)
    for i, (dx, dy) in enumerate(shifts.tolist()):
        masks[i] = upsampled[i, :, dy:dy + size, dx:dx + size]
    return masks


@torch.no_grad()
def rise_saliency(model, x, classes, n_masks, p=0.5, batch_size=100, seed=0):
    """
    Monte Carlo estimate of Eq. (6):
        S(lambda) = 1 / (N * E[M]) * sum_i f(I * M_i) * M_i(lambda)
    Masks are generated batch by batch, so memory stays small even for N = 8000.
    Returns one map per class in `classes`: [len(classes), H, W].
    """
    g = torch.Generator().manual_seed(seed)
    sal = torch.zeros(len(classes), INPUT_SIZE * INPUT_SIZE, device=x.device)
    done = 0
    while done < n_masks:
        b = min(batch_size, n_masks - done)
        m = generate_masks(b, g, p=p).to(x.device)            # [b, 1, H, W]
        masked = x * m                                        # element-wise: hide parts of the image
        probs = F.softmax(model(normalize(masked)), dim=1)    # black box: only the output is used
        scores = probs[:, classes]                            # [b, k] score of each class we explain
        sal += scores.T @ m.view(b, -1)                       # add each mask, weighted by its score
        done += b
        print(f"\r  {done}/{n_masks} masks", end="", flush=True)
    print()
    return (sal / (n_masks * p)).view(len(classes), INPUT_SIZE, INPUT_SIZE)


def main():
    ap = argparse.ArgumentParser(description="RISE saliency maps for a pretrained ResNet50")
    ap.add_argument("image")
    ap.add_argument("--masks", type=int, default=4000, help="number of random masks N")
    ap.add_argument("--topk", type=int, default=2, help="how many top classes to explain")
    ap.add_argument("--batch", type=int, default=100)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out", default="rise_result.png")
    args = ap.parse_args()

    if torch.cuda.is_available():
        device = "cuda"
    elif torch.backends.mps.is_available():   # Apple Silicon
        device = "mps"
    else:
        device = "cpu"
    model, categories = load_model(device)
    original, x = load_image(args.image)
    x = x.to(device)

    # 1. What does the model see in the unmasked image?
    with torch.no_grad():
        probs = F.softmax(model(normalize(x)), dim=1)[0]
    top_p, top_c = probs.topk(args.topk)
    print("Top predictions:")
    for pr, c in zip(top_p.tolist(), top_c.tolist()):
        print(f"  {categories[c]:<25} {pr:6.1%}")

    # 2. Probe the black box with N randomly masked versions of the image
    print(f"Running {args.masks} masked images through the model on {device} ...")
    sal = rise_saliency(model, x, top_c.tolist(), args.masks, batch_size=args.batch, seed=args.seed).cpu()

    # 3. Plot: original image plus one importance map per top class
    w, h = original.size
    fig, axes = plt.subplots(1, args.topk + 1, figsize=(4 * (args.topk + 1), 4 * h / w + 0.6))
    axes[0].imshow(original)
    axes[0].set_title("Input", fontsize=13)
    for i, (ax, pr, c) in enumerate(zip(axes[1:], top_p.tolist(), top_c.tolist())):
        m = F.interpolate(sal[i][None, None], size=(h, w), mode="bilinear", align_corners=False)[0, 0]
        ax.imshow(original)
        ax.imshow(m.numpy(), cmap="jet", alpha=0.5)
        ax.set_title(f"'{categories[c]}'  ({pr:.0%})", fontsize=13)
    for ax in axes:
        ax.axis("off")
    plt.tight_layout()
    plt.savefig(args.out, dpi=150, bbox_inches="tight")
    print(f"Saved {args.out}")


if __name__ == "__main__":
    main()