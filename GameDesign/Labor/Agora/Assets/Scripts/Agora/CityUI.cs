using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.InputSystem.UI;

// Spiel-Menü (uGUI), wird beim Start aus Code aufgebaut.
//   Links: Menü mit Reitern  Felder | Straßen | Bewohner | System
//   Oben rechts: Stadtkasse, Bewohner, Zufriedenheit, Pause
//   Unten Mitte: kurze Hinweise (z. B. "Nicht genug Geld")
public class CityUI : MonoBehaviour
{
    [Header("Farben")]
    public Color panelColor = new Color(0.11f, 0.12f, 0.15f, 0.94f);
    public Color buttonColor = new Color(0.21f, 0.23f, 0.28f, 1f);
    public Color accentColor = new Color(1f, 0.75f, 0.2f, 1f);
    public Color textColor = new Color(0.93f, 0.93f, 0.95f, 1f);
    public Color dimTextColor = new Color(0.65f, 0.67f, 0.72f, 1f);

    private Economy economy;
    private CityGrid grid;
    private BuildTool tool;

    private RectTransform canvasRoot;
    private readonly List<GameObject> pages = new List<GameObject>();
    private readonly List<UnityEngine.UI.Image> tabImages = new List<UnityEngine.UI.Image>();
    private readonly Dictionary<ZoneDefinition, UnityEngine.UI.Image> zoneButtons = new Dictionary<ZoneDefinition, UnityEngine.UI.Image>();
    private UnityEngine.UI.Image clearZoneButton, buildRoadButton, removeRoadButton, expandButton;
    private TextMeshProUGUI expandLabel;
    private RectTransform fieldList;
    private int currentTab = -1;

    // Anzeigen, die regelmäßig aktualisiert werden
    private Bar overallBar, unemployedBar, jobsBar, tradeBar;
    private readonly List<FieldRow> fieldRows = new List<FieldRow>();
    private TextMeshProUGUI residentsValue, taxValue, wageValue, priceValue, stockValue, firmValue, treasuryValue;
    private TextMeshProUGUI topTreasury, topResidents, pauseLabel, toast;
    private Bar topSatisfaction;
    private float refreshTimer;

    // Finanzen
    private readonly List<FinanceSection> financeSections = new List<FinanceSection>();
    private readonly List<RectTransform> historyColumns = new List<RectTransform>();
    private TextMeshProUGUI historyRange;
    private UnityEngine.UI.Image windowButton, totalButton;
    private bool showTotal;

    // ---------------------------------------------------------------- Aufbau

    void Start()
    {
        economy = GetComponent<Economy>();
        grid = GetComponent<CityGrid>();
        tool = GetComponent<BuildTool>();
        tool.Changed += RefreshToolHighlight;
        grid.LandChanged += RebuildFieldRows;

        EnsureEventSystem();
        BuildCanvas();
        BuildSidePanel();
        BuildTopBar();
        BuildToast();
        SelectTab(0);
        Refresh();
    }

    void OnDestroy()
    {
        if (tool != null) tool.Changed -= RefreshToolHighlight;
        if (grid != null) grid.LandChanged -= RebuildFieldRows;
    }

    void EnsureEventSystem()
    {
        if (FindAnyObjectByType<EventSystem>() != null) return;
        GameObject es = new GameObject("EventSystem", typeof(EventSystem));
        es.AddComponent<InputSystemUIInputModule>().AssignDefaultActions();
    }

    void BuildCanvas()
    {
        GameObject go = new GameObject("Menue", typeof(RectTransform), typeof(Canvas),
            typeof(UnityEngine.UI.CanvasScaler), typeof(UnityEngine.UI.GraphicRaycaster));
        go.GetComponent<Canvas>().renderMode = RenderMode.ScreenSpaceOverlay;
        var scaler = go.GetComponent<UnityEngine.UI.CanvasScaler>();
        scaler.uiScaleMode = UnityEngine.UI.CanvasScaler.ScaleMode.ScaleWithScreenSize;
        scaler.referenceResolution = new Vector2(1920, 1080);
        scaler.matchWidthOrHeight = 0.5f;
        canvasRoot = go.GetComponent<RectTransform>();
    }

    void BuildSidePanel()
    {
        // Panel oben links, Höhe passt sich dem Inhalt an
        RectTransform panel = NewRect("Menue Panel", canvasRoot);
        panel.anchorMin = panel.anchorMax = panel.pivot = new Vector2(0f, 1f);
        panel.anchoredPosition = new Vector2(20f, -20f);
        panel.sizeDelta = new Vector2(450f, 0f);
        AddImage(panel, panelColor, true);
        AddVertical(panel, 10f, new RectOffset(14, 14, 14, 16));
        var fitter = panel.gameObject.AddComponent<UnityEngine.UI.ContentSizeFitter>();
        fitter.verticalFit = UnityEngine.UI.ContentSizeFitter.FitMode.PreferredSize;

        Label(panel, "AGORA", 30, accentColor, FontStyles.Bold, 38f);

        // Reiter
        RectTransform tabs = NewRect("Reiter", panel);
        var hlg = tabs.gameObject.AddComponent<UnityEngine.UI.HorizontalLayoutGroup>();
        hlg.spacing = 6f;
        hlg.childControlWidth = hlg.childControlHeight = true;
        hlg.childForceExpandWidth = true;
        hlg.childForceExpandHeight = true;
        Layout(tabs, 44f);
        string[] names = { "Felder", "Straßen", "Bewohner", "Finanzen", "System" };
        for (int i = 0; i < names.Length; i++)
        {
            int index = i;
            var img = MakeButton(tabs, names[i], 16, () => SelectTab(index), 44f);
            tabImages.Add(img);
        }

        pages.Add(BuildFieldsPage(panel));
        pages.Add(BuildRoadsPage(panel));
        pages.Add(BuildResidentsPage(panel));
        pages.Add(BuildFinancePage(panel));
        pages.Add(BuildSystemPage(panel));
    }

    GameObject BuildFieldsPage(RectTransform parent)
    {
        RectTransform page = Page(parent, "Felder");
        Label(page, "Typ wählen, dann auf ein graues Feld klicken.", 18, dimTextColor, FontStyles.Normal, 26f);

        foreach (ZoneDefinition zone in tool.zones)
        {
            ZoneDefinition z = zone;
            RectTransform row = NewRect(z.displayName, page);
            Layout(row, 56f);
            var bg = AddImage(row, buttonColor, true);
            var button = row.gameObject.AddComponent<UnityEngine.UI.Button>();
            button.targetGraphic = bg;
            button.onClick.AddListener(() => tool.SelectZone(z));
            zoneButtons[z] = bg;

            // Farbfeld
            RectTransform swatch = NewRect("Farbe", row);
            swatch.anchorMin = swatch.anchorMax = swatch.pivot = new Vector2(0f, 0.5f);
            swatch.anchoredPosition = new Vector2(10f, 0f);
            swatch.sizeDelta = new Vector2(36f, 36f);
            AddImage(swatch, z.groundColor, false);

            TextMeshProUGUI name = Label(row, z.displayName, 22, textColor, FontStyles.Bold, 0f);
            Stretch(name.rectTransform, 58f, 90f);
            TextMeshProUGUI cost = Label(row, z.buildCost.ToString(), 20, accentColor, FontStyles.Normal, 0f);
            Stretch(cost.rectTransform, 0f, 12f);
            cost.alignment = TextAlignmentOptions.MidlineRight;
        }

        clearZoneButton = MakeButton(page, "Feld leeren", 20, tool.SelectClearZone, 44f);

        Label(page, "Stadt erweitern", 20, textColor, FontStyles.Bold, 30f);
        Label(page, "Angrenzendes Bauland (dunkel) zur Stadt hinzufügen.\nJede Erweiterung wird teurer.", 16, dimTextColor, FontStyles.Normal, 40f);
        expandButton = MakeButton(page, "", 20, tool.SelectExpand, 48f);
        expandLabel = expandButton.GetComponentInChildren<TextMeshProUGUI>();
        return page.gameObject;
    }

    GameObject BuildRoadsPage(RectTransform parent)
    {
        RectTransform page = Page(parent, "Strassen");
        Label(page, "Klicken und ziehen. Häuser entstehen nur an Straßen,\nauf Straßen laufen Bewohner schneller.", 18, dimTextColor, FontStyles.Normal, 48f);
        buildRoadButton = MakeButton(page, $"Straße bauen  ({economy.rules.roadCost:0} pro Stück)", 20, tool.SelectRoad, 48f);
        removeRoadButton = MakeButton(page, "Straße entfernen", 20, tool.SelectRemoveRoad, 44f);
        return page.gameObject;
    }

    GameObject BuildResidentsPage(RectTransform parent)
    {
        RectTransform page = Page(parent, "Bewohner");

        Label(page, "Zufriedenheit", 20, textColor, FontStyles.Bold, 26f);
        overallBar = MakeBar(page, 30f);
        residentsValue = Label(page, "", 18, dimTextColor, FontStyles.Normal, 24f);

        Label(page, "Arbeit", 20, textColor, FontStyles.Bold, 26f);
        unemployedBar = MakeBar(page, 22f);
        jobsBar = MakeBar(page, 22f);

        Label(page, "Felder", 20, textColor, FontStyles.Bold, 26f);
        fieldList = NewRect("Feldliste", page);
        AddVertical(fieldList, 4f, new RectOffset(0, 0, 0, 0));
        RebuildFieldRows();
        return page.gameObject;
    }

    // Eine Zeile pro Feld der Stadt; wird nach jeder Erweiterung neu aufgebaut
    void RebuildFieldRows()
    {
        if (fieldList == null) return;
        foreach (Transform child in fieldList) Destroy(child.gameObject);
        fieldRows.Clear();
        foreach (CityField f in grid.OwnedFields)
        {
            RectTransform row = NewRect(f.name, fieldList);
            Layout(row, 40f);
            AddVertical(row, 2f, new RectOffset(0, 0, 0, 0));
            fieldRows.Add(new FieldRow
            {
                field = f,
                title = Label(row, "", 16, textColor, FontStyles.Normal, 18f),
                bar = MakeBar(row, 18f)
            });
        }
        if (topTreasury != null) Refresh(); // beim ersten Aufbau existiert die Statusleiste noch nicht
    }

    GameObject BuildSystemPage(RectTransform parent)
    {
        RectTransform page = Page(parent, "System");
        Label(page, economy.rules.displayName, 22, accentColor, FontStyles.Bold, 30f);
        if (!string.IsNullOrEmpty(economy.rules.description))
            Label(page, economy.rules.description, 16, dimTextColor, FontStyles.Italic, 44f).textWrappingMode = TextWrappingModes.Normal;

        Label(page, "Regeln (gibst du vor)", 20, textColor, FontStyles.Bold, 26f);
        taxValue = Stepper(page, "Marktsteuer", () => economy.rules.taxRate = Mathf.Clamp01(economy.rules.taxRate - 0.05f),
                                                  () => economy.rules.taxRate = Mathf.Clamp01(economy.rules.taxRate + 0.05f));
        wageValue = Stepper(page, "Lohn pro Schicht", () => economy.rules.wage = Mathf.Max(0f, economy.rules.wage - 1f),
                                                       () => economy.rules.wage += 1f);

        Label(page, "Markt (entsteht von selbst)", 20, textColor, FontStyles.Bold, 26f);
        priceValue = ValueRow(page, "Preis einer Ware");
        stockValue = ValueRow(page, "Warenlager");
        firmValue = ValueRow(page, "Kapital der Firmen");
        treasuryValue = ValueRow(page, "Stadtkasse");
        tradeBar = MakeBar(page, 22f);
        return page.gameObject;
    }

    void BuildTopBar()
    {
        RectTransform bar = NewRect("Status", canvasRoot);
        bar.anchorMin = bar.anchorMax = bar.pivot = new Vector2(1f, 1f);
        bar.anchoredPosition = new Vector2(-20f, -20f);
        bar.sizeDelta = new Vector2(0f, 56f);
        AddImage(bar, panelColor, true);
        var hlg = bar.gameObject.AddComponent<UnityEngine.UI.HorizontalLayoutGroup>();
        hlg.padding = new RectOffset(16, 10, 8, 8);
        hlg.spacing = 18f;
        hlg.childAlignment = TextAnchor.MiddleLeft;
        hlg.childControlWidth = hlg.childControlHeight = true;
        hlg.childForceExpandWidth = hlg.childForceExpandHeight = false;
        var fit = bar.gameObject.AddComponent<UnityEngine.UI.ContentSizeFitter>();
        fit.horizontalFit = UnityEngine.UI.ContentSizeFitter.FitMode.PreferredSize;

        topTreasury = Label(bar, "", 22, accentColor, FontStyles.Bold, 40f);
        topResidents = Label(bar, "", 22, textColor, FontStyles.Normal, 40f);

        topSatisfaction = MakeBar(bar, 24f);
        topSatisfaction.root.GetComponent<UnityEngine.UI.LayoutElement>().preferredWidth = 160f;

        RectTransform pause = NewRect("Pause", bar);
        var pauseImg = AddImage(pause, buttonColor, true);
        var le = pause.gameObject.AddComponent<UnityEngine.UI.LayoutElement>();
        le.preferredWidth = 110f;
        le.preferredHeight = 40f;
        var b = pause.gameObject.AddComponent<UnityEngine.UI.Button>();
        b.targetGraphic = pauseImg;
        b.onClick.AddListener(() => Time.timeScale = Time.timeScale > 0f ? 0f : 1f);
        pauseLabel = Label(pause, "Pause", 20, textColor, FontStyles.Bold, 0f);
        Stretch(pauseLabel.rectTransform, 0f, 0f);
        pauseLabel.alignment = TextAlignmentOptions.Center;
    }

    void BuildToast()
    {
        RectTransform rt = NewRect("Hinweis", canvasRoot);
        rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(0.5f, 0f);
        rt.anchoredPosition = new Vector2(0f, 40f);
        rt.sizeDelta = new Vector2(900f, 50f);
        toast = rt.gameObject.AddComponent<TextMeshProUGUI>();
        toast.fontSize = 24;
        toast.alignment = TextAlignmentOptions.Center;
        toast.color = new Color(1f, 0.55f, 0.45f);
        toast.outlineWidth = 0.25f;
        toast.outlineColor = Color.black;
        toast.raycastTarget = false;
    }

    // ---------------------------------------------------------------- Verhalten

    void SelectTab(int index)
    {
        currentTab = index;
        for (int i = 0; i < pages.Count; i++) pages[i].SetActive(i == index);
        for (int i = 0; i < tabImages.Count; i++) tabImages[i].color = i == index ? accentColor : buttonColor;
        for (int i = 0; i < tabImages.Count; i++)
            tabImages[i].GetComponentInChildren<TextMeshProUGUI>().color = i == index ? Color.black : textColor;

        // Reiter "Straßen" -> direkt Straßen bauen; andere Reiter beenden das Bauen
        if (index == 1) tool.SelectRoad();
        else if (index != 0 || tool.CurrentMode == BuildTool.Mode.Road || tool.CurrentMode == BuildTool.Mode.RemoveRoad) tool.Cancel();
        RefreshToolHighlight();
    }

    void RefreshToolHighlight()
    {
        foreach (var pair in zoneButtons)
            pair.Value.color = tool.CurrentMode == BuildTool.Mode.Zone && tool.SelectedZone == pair.Key
                ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;
        if (expandButton != null)
            expandButton.color = tool.CurrentMode == BuildTool.Mode.Expand ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;
        if (clearZoneButton != null)
            clearZoneButton.color = tool.CurrentMode == BuildTool.Mode.ClearZone ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;
        if (buildRoadButton != null)
            buildRoadButton.color = tool.CurrentMode == BuildTool.Mode.Road ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;
        if (removeRoadButton != null)
            removeRoadButton.color = tool.CurrentMode == BuildTool.Mode.RemoveRoad ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;

        // Rechtsklick/Esc im Straßen-Reiter -> zurück zu den Feldern
        if (tool.CurrentMode == BuildTool.Mode.None && currentTab == 1) { currentTab = -1; SelectTab(0); }
    }

    void Update()
    {
        toast.text = Time.unscaledTime < tool.MessageUntil ? tool.Message : "";
        refreshTimer -= Time.unscaledDeltaTime;
        if (refreshTimer <= 0f)
        {
            refreshTimer = 0.25f;
            Refresh();
        }
    }

    void Refresh()
    {
        EconomySystem rules = economy.rules;
        int residents = grid.Residents.Count;
        float sat = grid.AverageSatisfaction();

        topTreasury.text = $"Kasse {economy.Treasury:0}";
        topResidents.text = $"Bewohner {residents}";
        topSatisfaction.Set(residents > 0 ? sat : 0f, residents > 0 ? $"{sat:P0} zufrieden" : "-", Resident.SatisfactionColor(sat));
        pauseLabel.text = Time.timeScale == 0f ? "Weiter" : "Pause";
        if (expandLabel != null)
            expandLabel.text = $"Stadt erweitern  ({grid.ExpansionPrice:0})";

        if (currentTab == 2)
        {
            overallBar.Set(residents > 0 ? sat : 0f, residents > 0 ? $"{sat:P0}" : "noch keine Bewohner", Resident.SatisfactionColor(sat));
            residentsValue.text = $"{residents} Bewohner";
            int unemployed = grid.Unemployed();
            unemployedBar.Set(residents > 0 ? (float)unemployed / residents : 0f, $"Arbeitslos: {unemployed}", new Color(0.85f, 0.35f, 0.3f));
            int jobs = grid.TotalJobs();
            jobsBar.Set(jobs > 0 ? (float)(jobs - grid.FreeJobs()) / jobs : 0f, $"Jobs besetzt: {jobs - grid.FreeJobs()} / {jobs}", new Color(0.85f, 0.7f, 0.25f));

            foreach (FieldRow row in fieldRows)
            {
                CityField f = row.field;
                int n = f.ResidentCount();
                row.title.text = f.DisplayName + (f.Is(ZoneKind.Wohnen) ? $"  ({n} Bewohner)" : "");
                if (f.Is(ZoneKind.Wohnen) && n > 0)
                {
                    float s = f.AverageSatisfaction();
                    row.bar.Set(s, $"{s:P0}", Resident.SatisfactionColor(s));
                }
                else
                    row.bar.Set(0f, f.Zone == null ? "leer" : (f.Is(ZoneKind.Wohnen) ? "noch keine Häuser" : "keine Bewohner"), Color.gray);
            }
        }
        else if (currentTab == 3)
        {
            RefreshFinance();
        }
        else if (currentTab == 4)
        {
            taxValue.text = $"{rules.taxRate:P0}";
            wageValue.text = $"{rules.wage:0}";
            priceValue.text = $"{economy.Price:0.00}";
            stockValue.text = $"{economy.GoodsStock}";
            firmValue.text = $"{economy.FirmCapital:0}";
            treasuryValue.text = $"{economy.Treasury:0}";
            int demand = economy.LastDemand;
            tradeBar.Set(demand > 0 ? (float)economy.LastSold / demand : 0f,
                $"Handel: {economy.LastSold} von {demand} Kaufwünschen erfüllt", new Color(0.35f, 0.6f, 0.9f));
        }
    }

    // ---------------------------------------------------------------- Bausteine

    // ---------------------------------------------------------------- Finanzen

    GameObject BuildFinancePage(RectTransform parent)
    {
        RectTransform page = Page(parent, "Finanzen");

        // Zeitraum umschalten
        RectTransform range = NewRect("Zeitraum", page);
        Layout(range, 34f);
        var hlg = range.gameObject.AddComponent<UnityEngine.UI.HorizontalLayoutGroup>();
        hlg.spacing = 6f;
        hlg.childControlWidth = hlg.childControlHeight = true;
        hlg.childForceExpandWidth = hlg.childForceExpandHeight = true;
        windowButton = MakeButton(range, $"Letzte {economy.financeWindowTicks} Ticks", 16, () => { showTotal = false; RefreshFinance(); }, 34f);
        totalButton = MakeButton(range, "Seit Spielbeginn", 16, () => { showTotal = true; RefreshFinance(); }, 34f);

        financeSections.Add(MakeFinanceSection(page, economy.City,
            new[] { Ledger.Marktsteuer },
            new[] { Ledger.FelderAusweisen, Ledger.Strassenbau, Ledger.Erweiterung, Ledger.Unterhalt }));

        // Verlauf der Stadtkasse
        historyRange = Label(page, "", 15, dimTextColor, FontStyles.Normal, 20f);
        RectTransform chart = NewRect("Kassenverlauf", page);
        Layout(chart, 56f);
        AddImage(chart, new Color(0.06f, 0.06f, 0.08f, 1f), false);
        for (int i = 0; i < economy.historyLength; i++)
        {
            RectTransform col = NewRect("Saeule", chart);
            col.anchorMin = new Vector2((float)i / economy.historyLength, 0f);
            col.anchorMax = new Vector2((float)(i + 1) / economy.historyLength, 0f);
            col.offsetMin = new Vector2(0.5f, 0f);
            col.offsetMax = new Vector2(-0.5f, 0f);
            AddImage(col, accentColor, false);
            historyColumns.Add(col);
        }

        financeSections.Add(MakeFinanceSection(page, economy.Firms,
            new[] { Ledger.Verkaeufe },
            new[] { Ledger.Loehne, Ledger.Investitionen }));
        financeSections.Add(MakeFinanceSection(page, economy.Households,
            new[] { Ledger.Loehne },
            new[] { Ledger.Einkaeufe }));
        return page.gameObject;
    }

    FinanceSection MakeFinanceSection(RectTransform page, Ledger ledger, string[] income, string[] expenses)
    {
        var section = new FinanceSection { ledger = ledger };

        RectTransform header = NewRect(ledger.Name, page);
        Layout(header, 30f);
        TextMeshProUGUI title = Label(header, ledger.Name, 20, textColor, FontStyles.Bold, 0f);
        Stretch(title.rectTransform, 0f, 150f);
        section.balance = Label(header, "", 20, textColor, FontStyles.Bold, 0f);
        Stretch(section.balance.rectTransform, 150f, 0f);
        section.balance.alignment = TextAlignmentOptions.MidlineRight;

        foreach (string c in income) section.rows.Add(MakeFinanceRow(page, c, true));
        foreach (string c in expenses) section.rows.Add(MakeFinanceRow(page, c, false));
        return section;
    }

    FinanceRow MakeFinanceRow(RectTransform page, string category, bool income)
    {
        Bar bar = MakeBar(page, 24f);
        bar.label.text = category;
        TextMeshProUGUI value = Label(bar.root, "", 16, Color.white, FontStyles.Bold, 0f);
        Stretch(value.rectTransform, 8f, 8f);
        value.alignment = TextAlignmentOptions.MidlineRight;
        return new FinanceRow { category = category, income = income, bar = bar, value = value };
    }

    static readonly Color incomeColor = new Color(0.25f, 0.68f, 0.32f);
    static readonly Color expenseColor = new Color(0.78f, 0.27f, 0.22f);

    void RefreshFinance()
    {
        if (windowButton == null) return;
        windowButton.color = showTotal ? buttonColor : Color.Lerp(buttonColor, accentColor, 0.45f);
        totalButton.color = showTotal ? Color.Lerp(buttonColor, accentColor, 0.45f) : buttonColor;

        foreach (FinanceSection s in financeSections)
        {
            // Balkenlänge relativ zum größten Posten des Abschnitts
            float max = 0.01f;
            foreach (FinanceRow r in s.rows)
                max = Mathf.Max(max, Mathf.Abs(Amount(s.ledger, r.category)));

            foreach (FinanceRow r in s.rows)
            {
                float amount = Amount(s.ledger, r.category);
                // Einnahmen positiv, Ausgaben negativ gebucht. Für Bewohner sind Löhne
                // Einnahme, für Firmen Ausgabe – das steckt im Vorzeichen der Buchung.
                r.bar.Set(Mathf.Abs(amount) / max, r.category, r.income ? incomeColor : expenseColor);
                r.value.text = Signed(amount);
            }

            float balance = showTotal ? s.ledger.TotalBalance() : s.ledger.WindowBalance();
            s.balance.text = "Bilanz " + Signed(balance);
            s.balance.color = balance >= 0f ? new Color(0.45f, 0.85f, 0.5f) : new Color(1f, 0.45f, 0.4f);
        }

        // Kassenverlauf
        var history = economy.TreasuryHistory;
        float lo = float.MaxValue, hi = float.MinValue;
        foreach (float v in history) { lo = Mathf.Min(lo, v); hi = Mathf.Max(hi, v); }
        lo = Mathf.Min(lo, 0f);
        float span = Mathf.Max(hi - lo, 1f);
        int offset = historyColumns.Count - history.Count;
        for (int i = 0; i < historyColumns.Count; i++)
        {
            int h = i - offset;
            float k = h >= 0 ? (history[h] - lo) / span : 0f;
            historyColumns[i].anchorMax = new Vector2(historyColumns[i].anchorMax.x, Mathf.Max(k, 0.02f));
            historyColumns[i].gameObject.SetActive(h >= 0);
        }
        historyRange.text = history.Count > 0 ? $"Verlauf Stadtkasse (letzte {history.Count} Ticks): jetzt {history[history.Count - 1]:0}, max {hi:0}" : "";
    }

    float Amount(Ledger ledger, string category) => showTotal ? ledger.Total(category) : ledger.Window(category);

    static string Signed(float v) => (v >= 0f ? "+" : "-") + Mathf.Abs(v).ToString("0");

    class FinanceSection
    {
        public Ledger ledger;
        public TextMeshProUGUI balance;
        public readonly List<FinanceRow> rows = new List<FinanceRow>();
    }

    class FinanceRow
    {
        public string category;
        public bool income;
        public Bar bar;
        public TextMeshProUGUI value;
    }

    class Bar
    {
        public RectTransform root;
        public RectTransform fill;
        public UnityEngine.UI.Image fillImage;
        public TextMeshProUGUI label;

        public void Set(float value, string text, Color color)
        {
            fill.anchorMax = new Vector2(Mathf.Clamp01(value), 1f);
            fillImage.color = color;
            label.text = text;
        }
    }

    class FieldRow
    {
        public CityField field;
        public TextMeshProUGUI title;
        public Bar bar;
    }

    RectTransform Page(RectTransform parent, string name)
    {
        RectTransform page = NewRect("Seite " + name, parent);
        AddVertical(page, 8f, new RectOffset(0, 0, 6, 0));
        return page;
    }

    Bar MakeBar(RectTransform parent, float height)
    {
        RectTransform root = NewRect("Balken", parent);
        Layout(root, height);
        AddImage(root, new Color(0.06f, 0.06f, 0.08f, 1f), false);

        RectTransform fill = NewRect("Fuellung", root);
        fill.anchorMin = Vector2.zero;
        fill.anchorMax = new Vector2(0f, 1f);
        fill.offsetMin = fill.offsetMax = Vector2.zero;
        var fillImage = AddImage(fill, accentColor, false);

        TextMeshProUGUI label = Label(root, "", height > 26f ? 20 : 16, Color.white, FontStyles.Bold, 0f);
        Stretch(label.rectTransform, 8f, 8f);
        // Keine Textkontur: outlineWidth braucht ein initialisiertes Material und schlägt
        // fehl, wenn der Balken in einem ausgeblendeten Reiter erzeugt wird.
        return new Bar { root = root, fill = fill, fillImage = fillImage, label = label };
    }

    TextMeshProUGUI Stepper(RectTransform parent, string title, System.Action minus, System.Action plus)
    {
        RectTransform row = NewRect(title, parent);
        Layout(row, 40f);
        TextMeshProUGUI t = Label(row, title, 19, textColor, FontStyles.Normal, 0f);
        Stretch(t.rectTransform, 0f, 160f);

        RectTransform m = SmallButton(row, "-", -112f, () => { minus(); Refresh(); });
        RectTransform p = SmallButton(row, "+", 0f, () => { plus(); Refresh(); });

        TextMeshProUGUI value = Label(row, "", 20, accentColor, FontStyles.Bold, 0f);
        value.rectTransform.anchorMin = value.rectTransform.anchorMax = value.rectTransform.pivot = new Vector2(1f, 0.5f);
        value.rectTransform.anchoredPosition = new Vector2(-44f, 0f);
        value.rectTransform.sizeDelta = new Vector2(64f, 40f);
        value.alignment = TextAlignmentOptions.Center;
        return value;
    }

    RectTransform SmallButton(RectTransform parent, string text, float x, System.Action onClick)
    {
        RectTransform rt = NewRect("Knopf " + text, parent);
        rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(1f, 0.5f);
        rt.anchoredPosition = new Vector2(x, 0f);
        rt.sizeDelta = new Vector2(40f, 36f);
        var img = AddImage(rt, buttonColor, true);
        var b = rt.gameObject.AddComponent<UnityEngine.UI.Button>();
        b.targetGraphic = img;
        b.onClick.AddListener(() => onClick());
        TextMeshProUGUI l = Label(rt, text, 24, textColor, FontStyles.Bold, 0f);
        Stretch(l.rectTransform, 0f, 0f);
        l.alignment = TextAlignmentOptions.Center;
        return rt;
    }

    TextMeshProUGUI ValueRow(RectTransform parent, string title)
    {
        RectTransform row = NewRect(title, parent);
        Layout(row, 26f);
        TextMeshProUGUI t = Label(row, title, 18, dimTextColor, FontStyles.Normal, 0f);
        Stretch(t.rectTransform, 0f, 120f);
        TextMeshProUGUI v = Label(row, "", 19, textColor, FontStyles.Bold, 0f);
        Stretch(v.rectTransform, 200f, 0f);
        v.alignment = TextAlignmentOptions.MidlineRight;
        return v;
    }

    UnityEngine.UI.Image MakeButton(RectTransform parent, string text, int size, System.Action onClick, float height)
    {
        RectTransform rt = NewRect(text, parent);
        Layout(rt, height);
        var img = AddImage(rt, buttonColor, true);
        var b = rt.gameObject.AddComponent<UnityEngine.UI.Button>();
        b.targetGraphic = img;
        b.onClick.AddListener(() => onClick());
        TextMeshProUGUI l = Label(rt, text, size, textColor, FontStyles.Bold, 0f);
        Stretch(l.rectTransform, 4f, 4f);
        l.alignment = TextAlignmentOptions.Center;
        return img;
    }

    TextMeshProUGUI Label(RectTransform parent, string text, int size, Color color, FontStyles style, float height)
    {
        RectTransform rt = NewRect("Text", parent);
        if (height > 0f) Layout(rt, height);
        var t = rt.gameObject.AddComponent<TextMeshProUGUI>();
        t.text = text;
        t.fontSize = size;
        t.color = color;
        t.fontStyle = style;
        t.alignment = TextAlignmentOptions.MidlineLeft;
        t.raycastTarget = false;
        t.textWrappingMode = TextWrappingModes.NoWrap;
        return t;
    }

    static RectTransform NewRect(string name, Transform parent)
    {
        GameObject go = new GameObject(name, typeof(RectTransform));
        go.transform.SetParent(parent, false);
        return go.GetComponent<RectTransform>();
    }

    static UnityEngine.UI.Image AddImage(RectTransform rt, Color color, bool raycast)
    {
        var img = rt.gameObject.AddComponent<UnityEngine.UI.Image>();
        img.color = color;
        img.raycastTarget = raycast;
        return img;
    }

    static UnityEngine.UI.VerticalLayoutGroup AddVertical(RectTransform rt, float spacing, RectOffset padding)
    {
        var vlg = rt.gameObject.AddComponent<UnityEngine.UI.VerticalLayoutGroup>();
        vlg.spacing = spacing;
        vlg.padding = padding;
        vlg.childControlWidth = vlg.childControlHeight = true;
        vlg.childForceExpandWidth = true;
        vlg.childForceExpandHeight = false;
        return vlg;
    }

    static void Layout(RectTransform rt, float height)
    {
        if (!rt.TryGetComponent(out UnityEngine.UI.LayoutElement le))
            le = rt.gameObject.AddComponent<UnityEngine.UI.LayoutElement>();
        le.preferredHeight = height;
        le.minHeight = height;
    }

    // Füllt das Elternelement, mit Abstand links/rechts
    static void Stretch(RectTransform rt, float left, float right)
    {
        rt.anchorMin = Vector2.zero;
        rt.anchorMax = Vector2.one;
        rt.offsetMin = new Vector2(left, 0f);
        rt.offsetMax = new Vector2(-right, 0f);
    }
}
