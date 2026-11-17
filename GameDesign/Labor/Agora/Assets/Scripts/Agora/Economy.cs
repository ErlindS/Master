using System.Collections.Generic;
using UnityEngine;

// Der Markt: hält Stadtkasse, Firmenkapital, Warenlager und Preis.
// Alle Geldflüsse laufen hier durch, damit der Kreislauf sichtbar bleibt:
//   Firma --Lohn--> Bewohner --Kauf--> Firma (+ Steuer an die Stadt)
//   Stadt --Unterhalt--> Institutionen
[DefaultExecutionOrder(-100)] // vor CityGrid, damit Regeln und Kassenbücher bereitstehen
public class Economy : MonoBehaviour
{
    public EconomySystem rules;

    public float Treasury { get; private set; }
    public float FirmCapital { get; private set; }
    public int GoodsStock { get; private set; }
    public float Price { get; private set; }
    public int Tick { get; private set; }

    // Kennzahlen des letzten Ticks (für HUD)
    public int LastDemand { get; private set; }
    public int LastSold { get; private set; }
    public int LastShortage { get; private set; }

    public event System.Action Ticked;

    [Tooltip("Über wie viele Ticks die Finanzansicht summiert")]
    public int financeWindowTicks = 30;
    [Tooltip("Wie viele Ticks der Kassenverlauf zurückreicht")]
    public int historyLength = 60;

    // Kassenbücher: woher kommt das Geld, wohin geht es?
    public Ledger City { get; private set; }
    public Ledger Firms { get; private set; }
    public Ledger Households { get; private set; }
    public IReadOnlyList<float> TreasuryHistory => treasuryHistory;
    private readonly List<float> treasuryHistory = new List<float>();

    private int demand, sold, shortage;
    private float tickTimer;
    private CityGrid grid;

    void Awake()
    {
        grid = GetComponent<CityGrid>();
        // Kopie, damit Änderungen im Spiel (Steuer, Lohn) nicht das Asset überschreiben
        rules = rules != null ? Instantiate(rules) : ScriptableObject.CreateInstance<EconomySystem>();
        Treasury = rules.startTreasury;
        FirmCapital = rules.startFirmCapital;
        Price = rules.startPrice;
        City = new Ledger("Stadtkasse", financeWindowTicks);
        Firms = new Ledger("Firmen", financeWindowTicks);
        Households = new Ledger("Bewohner", financeWindowTicks);
        treasuryHistory.Add(Treasury);
    }

    void Update()
    {
        tickTimer += Time.deltaTime;
        if (tickTimer >= rules.tickSeconds)
        {
            tickTimer -= rules.tickSeconds;
            DoTick();
        }
    }

    // ---- Stadtkasse ----

    public bool TrySpend(float amount, string category)
    {
        if (Treasury < amount) return false;
        Treasury -= amount;
        City.Book(category, -amount);
        return true;
    }

    // ---- Firmen ----

    public bool TrySpendFirm(float amount, string category)
    {
        if (FirmCapital < amount) return false;
        FirmCapital -= amount;
        Firms.Book(category, -amount);
        return true;
    }

    public void RefundFirm(float amount, string category)
    {
        FirmCapital += amount;
        Firms.Book(category, amount);
    }

    // ---- Arbeit ----

    // Ein Arbeiter beendet eine Schicht: Firma stellt Waren her und zahlt Lohn,
    // soweit ihr Kapital reicht.
    public float PayShift()
    {
        GoodsStock += rules.goodsPerShift;
        float paid = Mathf.Min(rules.wage, FirmCapital);
        FirmCapital -= paid;
        Firms.Book(Ledger.Loehne, -paid);
        Households.Book(Ledger.Loehne, paid);
        return paid;
    }

    // ---- Handel auf der Agora ----

    // Bewohner will eine Ware kaufen. Gibt true zurück, wenn der Handel zustande kommt.
    public bool TryBuy(ref float money)
    {
        demand++;
        if (GoodsStock <= 0) { shortage++; return false; }
        if (money < Price) return false;

        money -= Price;
        GoodsStock--;
        float tax = Price * rules.taxRate;
        Treasury += tax;
        FirmCapital += Price - tax;
        City.Book(Ledger.Marktsteuer, tax);
        Firms.Book(Ledger.Verkaeufe, Price - tax);
        Households.Book(Ledger.Einkaeufe, -Price);
        sold++;
        return true;
    }

    // ---- Tick ----

    void DoTick()
    {
        Tick++;

        // Preisbildung: Knappheit -> teurer, Überschuss -> billiger
        if (!rules.fixedPrice)
        {
            if (shortage > 0)
                Price *= 1f + rules.priceAdjust;
            else if (GoodsStock > Mathf.Max(demand, 1) * 3)
                Price *= 1f - rules.priceAdjust;
            // Firmen verkaufen nicht unter ihren Herstellungskosten (Lohn pro Ware)
            float unitCost = rules.wage / Mathf.Max(1, rules.goodsPerShift);
            Price = Mathf.Clamp(Price, Mathf.Max(rules.minPrice, unitCost * 1.1f), Mathf.Max(rules.maxPrice, unitCost * 1.1f));
        }

        // Unterhalt der Felder (z. B. Institutionen)
        if (grid != null)
        {
            float upkeep = grid.TotalUpkeep();
            Treasury -= upkeep;
            City.Book(Ledger.Unterhalt, -upkeep);
        }

        LastDemand = demand;
        LastSold = sold;
        LastShortage = shortage;
        demand = sold = shortage = 0;

        if (grid != null)
            grid.UpdateGrowth(rules);

        City.CloseTick();
        Firms.CloseTick();
        Households.CloseTick();
        treasuryHistory.Add(Treasury);
        if (treasuryHistory.Count > historyLength) treasuryHistory.RemoveAt(0);

        Ticked?.Invoke();
    }
}
