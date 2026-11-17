using UnityEngine;

// Die Arten von Feldern, die der Spieler ausweisen kann.
public enum ZoneKind
{
    Wohnen,      // Häuser, hier entstehen Bewohner
    Industrie,   // Arbeitsplätze, hier werden Waren hergestellt
    Agora,       // Marktplatz, hier wird gehandelt
    Institution  // Tempel/Rathaus: kostet Unterhalt, macht Bewohner im Umkreis zufriedener
}

// Daten eines Feldtyps. Liegt als Asset im Projekt (Assets/Data/Zones),
// damit man Werte im Inspector ändern kann, ohne Code anzufassen.
// Neuer Feldtyp = neues Asset (Rechtsklick > Create > Agora > Feldtyp).
[CreateAssetMenu(menuName = "Agora/Feldtyp", fileName = "Zone")]
public class ZoneDefinition : ScriptableObject
{
    public ZoneKind kind;
    public string displayName = "Feld";
    [TextArea] public string description;

    [Header("Aussehen")]
    public Color groundColor = Color.gray;
    public Color buildingColor = Color.white;

    [Header("Kosten")]
    [Tooltip("Kosten aus der Stadtkasse beim Ausweisen")]
    public int buildCost = 50;
    [Tooltip("Unterhalt pro Tick aus der Stadtkasse")]
    public float upkeepPerTick = 0f;

    [Header("Kapazität")]
    [Tooltip("Wohnen: Bewohner pro Stockwerk eines Hauses. Industrie: Arbeitsplätze pro Werkstatt.")]
    public int capacityPerLevel = 2;
    [Tooltip("Maximale Stockwerke eines Hauses")]
    public int maxLevel = 3;

    [Header("Wirkung (Institution)")]
    [Tooltip("Umkreis in Zellen, in dem die Institution wirkt")]
    public float influenceRadius = 0f;
}
