using UnityEngine;

// "Das System": alle Regeln, nach denen gehandelt wird, an einem Ort.
// Die Bewohner und Firmen entscheiden nicht moralisch, sie folgen nur diesen Regeln.
// Ein anderes System (z. B. hohe Steuern, fester Preis) = ein anderes Asset.
[CreateAssetMenu(menuName = "Agora/Wirtschaftssystem", fileName = "Wirtschaftssystem")]
public class EconomySystem : ScriptableObject
{
    public string displayName = "Freier Markt";
    [TextArea] public string description;

    [Header("Zeit")]
    [Tooltip("Sekunden pro Wirtschafts-Tick")]
    public float tickSeconds = 1f;

    [Header("Startwerte")]
    public float startTreasury = 400f;
    [Tooltip("Startkapital aller Firmen zusammen")]
    public float startFirmCapital = 150f;
    public float startResidentMoney = 10f;

    [Header("Arbeit")]
    [Tooltip("Lohn pro Schicht")]
    public float wage = 5f;
    [Tooltip("Waren, die ein Arbeiter pro Schicht herstellt")]
    public int goodsPerShift = 2;
    [Tooltip("Dauer einer Schicht in Sekunden")]
    public float shiftSeconds = 3f;

    [Tooltip("Kapital, das eine Firma für eine neue Werkstatt ausgibt")]
    public float investCost = 60f;

    [Header("Bauen")]
    [Tooltip("Kosten pro Straßenstück")]
    public float roadCost = 5f;
    [Tooltip("Kosten der ersten Stadterweiterung um ein Feld")]
    public float expansionBaseCost = 150f;
    [Tooltip("Jede weitere Erweiterung kostet so viel mal mehr")]
    public float expansionCostGrowth = 1.4f;

    [Header("Markt")]
    public float startPrice = 4f;
    [Tooltip("Preisänderung pro Tick bei Knappheit/Überschuss (0.05 = 5 %)")]
    public float priceAdjust = 0.05f;
    public bool fixedPrice = false;
    public float minPrice = 1f;
    public float maxPrice = 50f;
    [Tooltip("Steuer auf jeden Verkauf, geht in die Stadtkasse")]
    [Range(0f, 1f)] public float taxRate = 0.1f;

    [Header("Zufriedenheit und Wachstum")]
    [Range(0f, 1f)] public float growthThreshold = 0.6f;
    [Range(0f, 1f)] public float shrinkThreshold = 0.2f;
    [Range(0f, 1f)] public float growthChancePerTick = 0.15f;
    [Tooltip("Zufriedenheit, die eine Institution in Reichweite beim Heimkommen gibt")]
    public float institutionBonus = 0.05f;
}
