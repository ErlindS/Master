using System.Collections.Generic;

// Kassenbuch eines Teilnehmers (Stadt, Firmen, Bewohner).
// Jede Geldbewegung wird mit einer Kategorie gebucht: positiv = Einnahme, negativ = Ausgabe.
// Ausgewertet wird über die letzten N Ticks (gleitendes Fenster) und seit Spielbeginn.
public class Ledger
{
    // Kategorien (an einer Stelle, damit Buchung und Anzeige dieselben Namen benutzen)
    public const string Marktsteuer = "Marktsteuer";
    public const string FelderAusweisen = "Felder ausweisen";
    public const string Strassenbau = "Straßenbau";
    public const string Erweiterung = "Stadterweiterung";
    public const string Unterhalt = "Unterhalt Institutionen";
    public const string Verkaeufe = "Verkäufe auf der Agora";
    public const string Loehne = "Löhne";
    public const string Investitionen = "Investitionen (Werkstätten)";
    public const string Einkaeufe = "Einkäufe auf der Agora";

    public readonly string Name;
    public readonly int WindowTicks;

    private readonly Dictionary<string, float> thisTick = new Dictionary<string, float>();
    private readonly Queue<Dictionary<string, float>> past = new Queue<Dictionary<string, float>>();
    private readonly Dictionary<string, float> window = new Dictionary<string, float>();
    private readonly Dictionary<string, float> total = new Dictionary<string, float>();

    public Ledger(string name, int windowTicks)
    {
        Name = name;
        WindowTicks = windowTicks;
    }

    public void Book(string category, float amount)
    {
        if (amount == 0f) return;
        Add(thisTick, category, amount);
        Add(total, category, amount);
    }

    // Am Ende jedes Ticks: aktuellen Tick ins Fenster schieben, ältesten herausnehmen
    public void CloseTick()
    {
        var closed = new Dictionary<string, float>(thisTick);
        past.Enqueue(closed);
        foreach (var pair in closed) Add(window, pair.Key, pair.Value);
        thisTick.Clear();

        while (past.Count > WindowTicks)
            foreach (var pair in past.Dequeue()) Add(window, pair.Key, -pair.Value);
    }

    public float Window(string category) => window.TryGetValue(category, out float v) ? v : 0f;
    public float Total(string category) => total.TryGetValue(category, out float v) ? v : 0f;

    public float WindowBalance()
    {
        float sum = 0f;
        foreach (float v in window.Values) sum += v;
        return sum;
    }

    public float TotalBalance()
    {
        float sum = 0f;
        foreach (float v in total.Values) sum += v;
        return sum;
    }

    static void Add(Dictionary<string, float> d, string key, float value)
    {
        d.TryGetValue(key, out float old);
        d[key] = old + value;
    }
}
