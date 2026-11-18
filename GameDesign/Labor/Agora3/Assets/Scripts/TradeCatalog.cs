using System.Collections.Generic;
using UnityEngine;

// Alle handelbaren Dinge an einem Ort:
//   Teile    – gibt es am Markt (rosa) zu kaufen oder die Maschine (M) baut sie langsam selbst
//   Produkte – werden an der Maschine aus Teilen zusammengebaut und an die Kunden verkauft
public static class TradeCatalog
{
    public class Part
    {
        public string name;
        public Color color;
        public int price; // Preis am Markt
    }

    public class Product
    {
        public string name;
        public Color color;
        public string[] parts;  // Rezept (Teile dürfen mehrfach vorkommen)
        public int unlockAfter; // erst gewünscht, nachdem so viele Kunden bedient wurden
    }

    public static readonly List<Part> Parts = new List<Part>
    {
        new Part { name = "Holz",  color = new Color(0.62f, 0.42f, 0.24f), price = 2 },
        new Part { name = "Eisen", color = new Color(0.36f, 0.50f, 0.66f), price = 3 },
        new Part { name = "Stoff", color = new Color(0.66f, 0.30f, 0.66f), price = 2 },
        new Part { name = "Glas",  color = new Color(0.35f, 0.85f, 0.90f), price = 3 },
    };

    public static readonly List<Product> Products = new List<Product>
    {
        new Product { name = "Hammer",  color = new Color(0.95f, 0.75f, 0.20f), parts = new[] { "Holz", "Eisen" },          unlockAfter = 0 },
        new Product { name = "Fahne",   color = new Color(0.95f, 0.45f, 0.30f), parts = new[] { "Holz", "Stoff" },          unlockAfter = 0 },
        new Product { name = "Laterne", color = new Color(1.00f, 0.95f, 0.55f), parts = new[] { "Eisen", "Glas" },          unlockAfter = 2 },
        new Product { name = "Fenster", color = new Color(0.70f, 0.95f, 1.00f), parts = new[] { "Holz", "Glas" },           unlockAfter = 3 },
        new Product { name = "Kutsche", color = new Color(0.85f, 0.25f, 0.25f), parts = new[] { "Holz", "Holz", "Eisen" },  unlockAfter = 5 },
        new Product { name = "Zelt",    color = new Color(0.45f, 0.75f, 0.35f), parts = new[] { "Stoff", "Stoff", "Holz" }, unlockAfter = 6 },
        new Product { name = "Spiegel", color = new Color(0.90f, 0.90f, 0.95f), parts = new[] { "Glas", "Glas", "Eisen" },  unlockAfter = 8 },
    };

    public static Part FindPart(string name) => Parts.Find(p => p.name == name);
    public static Product FindProduct(string name) => Products.Find(p => p.name == name);
    public static bool IsPart(string name) => FindPart(name) != null;

    public static Color ColorOf(string item)
    {
        Part part = FindPart(item);
        if (part != null) return part.color;
        Product product = FindProduct(item);
        return product != null ? product.color : Color.white;
    }

    // Was das Produkt im Laden wert ist (Summe der Teilpreise)
    public static int PartCost(Product product)
    {
        int sum = 0;
        foreach (string p in product.parts) sum += FindPart(p).price;
        return sum;
    }

    // Farbiger Name für TextMeshPro-Rich-Text
    public static string Colored(string item) =>
        $"<color=#{ColorUtility.ToHtmlStringRGB(ColorOf(item))}>{item}</color>";

    // "Holz + Eisen" (farbig)
    public static string RecipeText(Product product)
    {
        List<string> names = new List<string>();
        foreach (string p in product.parts) names.Add(Colored(p));
        return string.Join(" + ", names);
    }
}
