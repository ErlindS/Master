using System.Collections.Generic;
using TMPro;
using UnityEngine;

// Eines der großen Felder. Anfangs gehören der Stadt nur die grauen Startfelder.
// Angrenzendes Bauland kann gegen Geld zur Stadt hinzugefügt werden (Erweiterung).
// Eigene Felder bekommen vom Spieler einen Feldtyp, dadurch eine Farbe,
// und Gebäude entstehen darauf von selbst.
public class CityField : MonoBehaviour
{
    public int GridX { get; private set; }
    public int GridY { get; private set; }
    public bool Owned { get; private set; }
    public int Number { get; private set; }
    public ZoneDefinition Zone { get; private set; }
    public List<Cell> Cells { get; private set; }
    public readonly List<Building> Buildings = new List<Building>();
    public float WorldSize { get; private set; }

    public bool Is(ZoneKind kind) => Zone != null && Zone.kind == kind;
    public string DisplayName => Owned ? $"Feld {Number}" + (Zone != null ? $" · {Zone.displayName}" : "") : "Bauland";

    private CityGrid grid;
    private Renderer ground;
    private Color emptyColor, unownedColor;
    private TextMeshPro priceLabel;

    public void Init(CityGrid owner, int gx, int gy, List<Cell> cells, float worldSize, Color color, Color unowned)
    {
        grid = owner;
        GridX = gx;
        GridY = gy;
        Cells = cells;
        WorldSize = worldSize;
        emptyColor = color;
        unownedColor = unowned;

        GameObject plate = GameObject.CreatePrimitive(PrimitiveType.Cube);
        plate.name = "Boden";
        Destroy(plate.GetComponent<Collider>());
        plate.transform.SetParent(transform, false);
        plate.transform.localPosition = new Vector3(0f, 0.02f, 0f);
        plate.transform.localScale = new Vector3(worldSize - 0.1f, 0.04f, worldSize - 0.1f);
        ground = plate.GetComponent<Renderer>();

        // Preisschild auf dem Bauland (liegt flach, lesbar von der Kamera aus)
        GameObject label = new GameObject("Preis", typeof(RectTransform));
        label.transform.SetParent(transform, false);
        label.transform.localPosition = new Vector3(0f, 0.08f, 0f);
        label.transform.localRotation = Quaternion.Euler(90f, 0f, 0f);
        priceLabel = label.AddComponent<TextMeshPro>();
        priceLabel.rectTransform.sizeDelta = new Vector2(worldSize, worldSize * 0.5f);
        priceLabel.alignment = TextAlignmentOptions.Center;
        priceLabel.fontSize = 9f;
        priceLabel.color = new Color(1f, 0.8f, 0.35f);
        priceLabel.textWrappingMode = TextWrappingModes.Normal;

        ShowHidden();
    }

    // ---- Besitz / Darstellung ----

    public void SetOwned(int number)
    {
        Owned = true;
        Number = number;
    }

    public void ShowOwned()
    {
        gameObject.SetActive(true);
        priceLabel.gameObject.SetActive(false);
        ground.sharedMaterial = CityMaterials.Get(Zone != null ? Zone.groundColor : emptyColor);
    }

    public void ShowForSale(float price)
    {
        gameObject.SetActive(true);
        priceLabel.gameObject.SetActive(true);
        priceLabel.text = $"Erweitern\n<b>{price:0}</b>";
        ground.sharedMaterial = CityMaterials.Get(unownedColor);
    }

    public void ShowHidden() => gameObject.SetActive(false);

    // Neuer Feldtyp: alte Gebäude verschwinden, das Feld wird umgefärbt
    public void SetZone(ZoneDefinition zone)
    {
        if (!Owned) return;
        foreach (Building b in Buildings.ToArray()) b.Remove();
        Buildings.Clear();
        Zone = zone;
        ShowOwned();
    }

    public bool HasRoadAccess()
    {
        foreach (Cell c in Cells)
            if (grid.IsNextToRoad(c)) return true;
        return false;
    }

    public int ResidentCount()
    {
        int n = 0;
        foreach (Building b in Buildings) if (b != null) n += b.Residents.Count;
        return n;
    }

    public float AverageSatisfaction()
    {
        float sum = 0f;
        int n = 0;
        foreach (Building b in Buildings)
        {
            if (b == null) continue;
            foreach (Resident r in b.Residents) { sum += r.Satisfaction; n++; }
        }
        return n == 0 ? 0f : sum / n;
    }
}
