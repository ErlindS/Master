using System.Collections;
using System.Collections.Generic;
using Unity.AI.Navigation;
using UnityEngine;
using UnityEngine.AI;
using TMPro;

// Handelsspiel auf dem Schiebebrett (Layout nach ideen/ersteIdee.png).
// Kunden (rote Kugeln) kommen herein, stellen sich an die Theke und wollen ein Produkt.
// Der Ball besorgt es auf einem von zwei Wegen:
//   1. Teile am MARKT (rosa) kaufen – schnell, kostet Münzen
//   2. Teile an der MASCHINE (M) selbst bauen lassen – kostenlos, aber langsam
// An der Maschine werden die Teile dann zum Produkt zusammengebaut, das der
// Ball zum Kunden bringt. Je mehr Kunden bedient sind, desto mehr kommen
// gleichzeitig, desto ungeduldiger sind sie und desto komplizierter die Wünsche.
// Das blaue Quadrat aus der Skizze ist ein Rundweg auf dem Boden, der alle
// Stationen verbindet: Kunden -> Maschine -> Markt -> Kunden.
// Gehört auf das Spielfeld-Objekt ("Walls"), zusammen mit PlaygroundMovement.
//
// Legende der Karte (20 x 20 Zeichen, 1 Zeichen = 1 Unity-Einheit):
//   #  graue Mauer
//   .  frei
//   M  Maschine (Teile bauen / Produkte zusammenbauen)
//   P  Markt (wird von oben nach unten gleichmäßig auf alle Teile aufgeteilt)
//   C  Platz an der Theke für einen Kunden
//   E  Eingang/Ausgang der Kunden
//   S  Startposition des Balls
[RequireComponent(typeof(PlaygroundMovement))]
public class LevelManager : MonoBehaviour
{
    [TextArea(20, 22)]
    public string shopMap =
        "...............#....\n" +
        "...............#....\n" +
        "...............#....\n" +
        "....................\n" +
        "....................\n" +
        "...#########...#....\n" +
        "....................\n" +
        "...MMMM.........PP..\n" +
        "...MMMM.........PP..\n" +
        "...MMMM.........PP..\n" +
        "................PP..\n" +
        "................PP..\n" +
        "..........S.....PP..\n" +
        "......#########.PP..\n" +
        "................PP..\n" +
        "....................\n" +
        "....................\n" +
        "....C..C..C..C......\n" +
        ".E................E.\n" +
        "....................";

    [Header("Kunden")]
    [Tooltip("Sekunden bis zum ersten Kunden")]
    public float firstCustomerDelay = 2f;
    [Tooltip("Wartezeit zwischen zwei neuen Kunden (am Anfang / später)")]
    public float spawnIntervalStart = 12f;
    public float spawnIntervalEnd = 5f;
    [Tooltip("Geduld eines Kunden an der Theke in Sekunden (am Anfang / später)")]
    public float patienceStart = 60f;
    public float patienceEnd = 30f;
    [Tooltip("Nach so vielen bedienten Kunden ist die volle Schwierigkeit erreicht")]
    public int servedForFullDifficulty = 12;
    [Tooltip("Gleichzeitige Kunden: am Anfang 1, alle n bedienten Kunden einer mehr")]
    public int servedPerExtraCustomer = 3;
    [Tooltip("Verkaufspreis = Teilpreise * Faktor (+ Trinkgeld für schnelle Bedienung)")]
    public float priceMultiplier = 2f;
    public float customerWalkSpeed = 3f;

    [Header("Maschine")]
    [Tooltip("So lange baut die Maschine ein fehlendes Teil (kostenlos)")]
    public float partBuildTime = 4f;
    [Tooltip("So lange dauert das Zusammenbauen eines Produkts")]
    public float assembleTime = 1.5f;
    public Color machineColor = new Color(0.25f, 0.45f, 0.85f);

    [Header("Markt")]
    public Color marketColor = new Color(1f, 0.55f, 0.8f);
    [Tooltip("Bleibt der Ball auf einem Stand, kauft er alle n Sekunden ein weiteres Teil")]
    public float marketRepeatTime = 1.2f;

    [Header("Eingang")]
    [Tooltip("So lange blinkt die Tür, bevor dort ein Kunde erscheint")]
    public float spawnWarningTime = 1f;
    public Color entryColor = new Color(1f, 0.8f, 0.2f);

    [Header("Aussehen")]
    public float wallHeight = 1f;
    [Tooltip("Dauer der Animation, mit der neue Mauern aus dem Boden wachsen")]
    public float wallRiseDuration = 0.5f;

    [Header("Bezugspunkt (fester Tisch unter dem Spielfeld)")]
    [Tooltip("Erzeugt beim Start einen festen, karierten Tisch unter dem Brett, damit man sieht, dass sich das Spielfeld bewegt")]
    public bool createReferenceTable = true;
    public float tableSize = 80f;
    public float tableDepth = 1f;
    public float tileSize = 2f;
    public Color tableColorA = new Color(0.30f, 0.32f, 0.36f);
    public Color tableColorB = new Color(0.40f, 0.42f, 0.47f);
    [Tooltip("Rahmen auf dem Tisch, der die Ruheposition des Bretts markiert")]
    public Color homeFrameColor = new Color(1f, 0.75f, 0.2f);

    [Header("Rundweg (blaues Quadrat)")]
    public bool createRing = true;
    [Tooltip("Zeilen/Spalten der Karte, auf denen das Quadrat liegt (0 = oben / links)")]
    public int ringTopRow = 3;
    public int ringBottomRow = 16;
    public int ringLeftCol = 2;
    public int ringRightCol = 15;
    public float ringWidth = 0.55f;
    public Color ringColor = new Color(0.15f, 0.55f, 1f);

    [Header("Referenzen (werden automatisch gesucht, wenn leer)")]
    public Transform ground;
    public Transform pickUpParent;
    public Material wallMaterial;
    public BallGameLogic ball;
    [Tooltip("Vorlage für alle Kunden (wird geklont)")]
    public EnemyMovement enemy;

    private const int GridSize = 20;
    private const float CellSize = 1f;

    private PlaygroundMovement playground;
    private NavMeshSurface navMeshSurface;
    private NavMeshDataInstance fixedNavMesh;
    private readonly bool[,] wallGrid = new bool[GridSize, GridSize];

    // Stationen
    private readonly bool[,] machineGrid = new bool[GridSize, GridSize];
    private readonly int[,] marketStall = new int[GridSize, GridSize]; // -1 = kein Stand, sonst Index in TradeCatalog.Parts
    private readonly List<Vector2Int> counterCells = new List<Vector2Int>();
    private readonly List<Vector2Int> entryCells = new List<Vector2Int>();
    private Vector3 machineCenterLocal;
    private TextMeshPro machineLabel;
    private Transform machineGear;

    // Kunden
    private readonly List<EnemyMovement> customers = new List<EnemyMovement>();
    private EnemyMovement[] counterTaken;
    private int pendingCustomers;
    private float spawnTimer;

    // Maschine
    private string machineJob;      // Name des Teils oder Produkts, das gerade entsteht
    private bool machineJobIsAssembly;
    private float machineProgress;

    // Markt
    private int lastStall = -1;
    private float stallTimer;

    private Material baseMaterial;

    // Wohin der Spieler als Nächstes sollte (für die Kamera), sonst null
    public Transform CurrentTarget { get; private set; }

    void Start()
    {
        playground = GetComponent<PlaygroundMovement>();
        if (!playground.enabled)
        {
            Debug.LogWarning("LevelManager: PlaygroundMovement war deaktiviert und wurde wieder eingeschaltet.", this);
            playground.enabled = true;
        }
        FindReferences();

        GameObject temp = GameObject.CreatePrimitive(PrimitiveType.Cube);
        baseMaterial = temp.GetComponent<Renderer>().sharedMaterial;
        Destroy(temp);

        // Alte, von Hand platzierte Hindernisse und Pickups ausblenden
        if (ground != null)
            foreach (Transform child in ground)
                child.gameObject.SetActive(false);
        if (pickUpParent != null)
            foreach (Transform child in pickUpParent)
                child.gameObject.SetActive(false);

        // Die NavMeshSurface hängt am beweglichen Brett. Aktiv würde sie ihr
        // NavMesh in jedem Frame versetzen -> wir setzen es selbst fest ein.
        if (navMeshSurface != null) navMeshSurface.enabled = false;

        // Der Kunde in der Szene dient nur als Vorlage für die Klone
        if (enemy != null) enemy.gameObject.SetActive(false);

        if (createReferenceTable) CreateReferenceTable();
        if (createRing) BuildRing();
        CreateOrdersUi();

        BuildShop();
        spawnTimer = firstCustomerDelay;
    }

    void FindReferences()
    {
        if (ground == null) ground = transform.Find("Ground");
        if (pickUpParent == null) pickUpParent = transform.Find("PickUpParent");
        if (wallMaterial == null)
        {
            Transform northWall = transform.Find("North Wall");
            if (northWall != null) wallMaterial = northWall.GetComponent<Renderer>().sharedMaterial;
        }
        if (ball == null) ball = FindAnyObjectByType<BallGameLogic>();
        if (enemy == null) enemy = FindAnyObjectByType<EnemyMovement>();
        if (ground != null) navMeshSurface = ground.GetComponent<NavMeshSurface>();
    }

    void OnDestroy()
    {
        fixedNavMesh.Remove();
    }

    // ================= Aufbau =================

    void BuildShop()
    {
        string[] rows = shopMap.Replace("\r", "").Trim('\n').Split('\n');
        List<Vector2Int> marketCells = new List<Vector2Int>();
        List<Vector2Int> machineCells = new List<Vector2Int>();
        List<GameObject> newWalls = new List<GameObject>();

        for (int r = 0; r < GridSize; r++)
            for (int c = 0; c < GridSize; c++)
                marketStall[r, c] = -1;

        for (int r = 0; r < rows.Length && r < GridSize; r++)
        {
            string row = rows[r];
            int c = 0;
            while (c < row.Length && c < GridSize)
            {
                char ch = row[c];
                if (ch == '#')
                {
                    // Nebeneinanderliegende Mauerstücke zu einem Block zusammenfassen
                    int start = c;
                    while (c < row.Length && c < GridSize && row[c] == '#')
                    {
                        wallGrid[r, c] = true;
                        c++;
                    }
                    newWalls.Add(SpawnWall(r, start, c - start));
                    continue;
                }
                switch (ch)
                {
                    case 'M': machineGrid[r, c] = true; machineCells.Add(new Vector2Int(r, c)); break;
                    case 'P': marketCells.Add(new Vector2Int(r, c)); break;
                    case 'C': counterCells.Add(new Vector2Int(r, c)); break;
                    case 'E': entryCells.Add(new Vector2Int(r, c)); break;
                    case 'S': playground.CenterBoardOn(CellToBoardLocal(r, c, 0.5f)); break;
                }
                c++;
            }
        }

        counterTaken = new EnemyMovement[counterCells.Count];
        if (counterCells.Count == 0) Debug.LogWarning("LevelManager: Die Karte hat keine Theke ('C')!");
        if (entryCells.Count == 0) Debug.LogWarning("LevelManager: Die Karte hat keinen Eingang ('E')!");

        RebuildNavMesh();
        foreach (GameObject wall in newWalls) StartCoroutine(RiseWall(wall.transform));

        BuildMachine(machineCells);
        BuildMarket(marketCells);
        BuildCounter();
        BuildEntries();
    }

    void BuildMachine(List<Vector2Int> cells)
    {
        if (cells.Count == 0) { Debug.LogWarning("LevelManager: Die Karte hat keine Maschine ('M')!"); return; }

        GetBounds(cells, out int r0, out int r1, out int c0, out int c1);
        Vector3 a = CellToBoardLocal(r0, c0, 0f), b = CellToBoardLocal(r1, c1, 0f);
        machineCenterLocal = (a + b) / 2f;
        Vector3 size = new Vector3(c1 - c0 + 1, 0f, r1 - r0 + 1) * CellSize;

        CreatePad("Machine", machineCenterLocal, size, machineColor, 0.4f);

        // Zahnrad (dreht sich, solange gearbeitet wird) – liegt am Rand, damit der Ball darüber rollt
        GameObject gear = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        gear.name = "MachineGear";
        Destroy(gear.GetComponent<Collider>());
        gear.GetComponent<Renderer>().material = MakeMaterial(machineColor * 0.7f, 0.2f);
        gear.transform.localScale = new Vector3(size.x * 0.6f, 0.04f, size.z * 0.6f);
        PlaceOnBoard(gear.transform, machineCenterLocal + new Vector3(0f, 0.08f, 0f));
        machineGear = gear.transform;

        machineLabel = CreateLabel("MachineLabel", machineCenterLocal + Vector3.up * 1.6f, 6f, Color.white);
        machineLabel.rectTransform.sizeDelta = new Vector2(6f, 2.5f);
        SetMachineLabel(null);
    }

    void BuildMarket(List<Vector2Int> cells)
    {
        if (cells.Count == 0) { Debug.LogWarning("LevelManager: Die Karte hat keinen Markt ('P')!"); return; }

        // Zeilen des Markts gleichmäßig auf die Teile verteilen
        List<int> rows = new List<int>();
        foreach (Vector2Int p in cells) if (!rows.Contains(p.x)) rows.Add(p.x);
        rows.Sort();
        int parts = TradeCatalog.Parts.Count;
        foreach (Vector2Int p in cells)
            marketStall[p.x, p.y] = Mathf.Min(parts - 1, rows.IndexOf(p.x) * parts / rows.Count);

        GetBounds(cells, out int r0, out int r1, out int c0, out int c1);
        Vector3 top = CellToBoardLocal(r0, c0, 0f), bottom = CellToBoardLocal(r1, c1, 0f);
        Vector3 center = (top + bottom) / 2f;
        CreatePad("Market", center, new Vector3(c1 - c0 + 1, 0f, r1 - r0 + 1) * CellSize, marketColor, 0.3f);

        // Je Stand: kleiner farbiger Würfel, Name und Preis
        for (int i = 0; i < parts; i++)
        {
            List<Vector2Int> stall = cells.FindAll(p => marketStall[p.x, p.y] == i);
            if (stall.Count == 0) continue;
            GetBounds(stall, out int sr0, out int sr1, out int sc0, out int sc1);
            Vector3 s = (CellToBoardLocal(sr0, sc0, 0f) + CellToBoardLocal(sr1, sc1, 0f)) / 2f;
            TradeCatalog.Part part = TradeCatalog.Parts[i];

            // Trennlinie zwischen den Ständen
            if (i > 0)
                CreatePad("StallDivider", new Vector3(s.x, 0f, CellToBoardLocal(sr0, 0, 0f).z + CellSize / 2f),
                          new Vector3((c1 - c0 + 1) * CellSize, 0f, 0.08f), Color.white, 0f, 0.035f);

            GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
            cube.name = "Stall " + part.name;
            Destroy(cube.GetComponent<Collider>());
            cube.GetComponent<Renderer>().material = MakeMaterial(part.color, 0.3f);
            cube.transform.localScale = Vector3.one * 0.45f;
            PlaceOnBoard(cube.transform, s + Vector3.up * 0.6f);
            cube.AddComponent<Rotator>();

            CreateLabel("StallLabel", s + new Vector3(2.2f, 0.3f, 0f), 3.5f, Color.white).text =
                $"{TradeCatalog.Colored(part.name)}\n<size=80%>{part.price} Münzen</size>";
        }
        CreateLabel("MarketLabel", new Vector3(center.x, 0f, top.z) + new Vector3(0f, 1.2f, 1f), 5f, marketColor).text = "<b>Markt</b>";
    }

    void BuildCounter()
    {
        foreach (Vector2Int cell in counterCells)
            CreatePad("CounterSpot", CellToBoardLocal(cell.x, cell.y, 0f), new Vector3(0.9f, 0f, 0.9f),
                      new Color(0.45f, 0.95f, 0.45f), 0.2f);
        if (counterCells.Count > 0)
        {
            GetBounds(counterCells, out int r0, out _, out int c0, out int c1);
            Vector3 mid = (CellToBoardLocal(r0, c0, 0f) + CellToBoardLocal(r0, c1, 0f)) / 2f;
            CreateLabel("CounterLabel", mid + new Vector3(0f, 0.1f, -1.6f), 4f, new Color(0.6f, 1f, 0.6f)).text = "Kunden";
        }
    }

    // Blaues Quadrat wie in der Skizze: vier Streifen auf dem Boden
    void BuildRing()
    {
        Vector3 tl = CellToBoardLocal(ringTopRow, ringLeftCol, 0f);
        Vector3 br = CellToBoardLocal(ringBottomRow, ringRightCol, 0f);
        float w = br.x - tl.x, h = tl.z - br.z;
        Vector3 mid = (tl + br) / 2f;
        float lift = 0.012f; // knapp unter den Stationen
        CreatePad("RingTop",    new Vector3(mid.x, 0f, tl.z), new Vector3(w + ringWidth, 0f, ringWidth), ringColor, 0.35f, lift);
        CreatePad("RingBottom", new Vector3(mid.x, 0f, br.z), new Vector3(w + ringWidth, 0f, ringWidth), ringColor, 0.35f, lift);
        CreatePad("RingLeft",   new Vector3(tl.x, 0f, mid.z), new Vector3(ringWidth, 0f, h + ringWidth), ringColor, 0.35f, lift);
        CreatePad("RingRight",  new Vector3(br.x, 0f, mid.z), new Vector3(ringWidth, 0f, h + ringWidth), ringColor, 0.35f, lift);
    }

    // Liste aller offenen Bestellungen oben rechts – so sieht man jederzeit, was die Kunden wollen
    TextMeshProUGUI ordersText;

    void CreateOrdersUi()
    {
        Canvas canvas = FindAnyObjectByType<Canvas>();
        if (canvas == null) return;
        GameObject go = new GameObject("OrdersText", typeof(RectTransform));
        go.transform.SetParent(canvas.transform, false);
        RectTransform rt = go.GetComponent<RectTransform>();
        rt.anchorMin = rt.anchorMax = rt.pivot = new Vector2(1f, 1f);
        rt.anchoredPosition = new Vector2(-15f, -15f);
        rt.sizeDelta = new Vector2(520f, 400f);
        ordersText = go.AddComponent<TextMeshProUGUI>();
        ordersText.alignment = TextAlignmentOptions.TopRight;
        ordersText.fontSize = 26f;
        ordersText.textWrappingMode = TextWrappingModes.NoWrap;
        ordersText.richText = true;
        ordersText.outlineWidth = 0.15f;
        ordersText.outlineColor = Color.black;
    }

    void UpdateOrdersUi()
    {
        if (ordersText == null) return;
        List<EnemyMovement> waiting = WaitingCustomers();
        if (waiting.Count == 0) { ordersText.text = "<b>Bestellungen</b>\n<color=#AAAAAA>keine – Kunde kommt gleich</color>"; return; }

        // Fertige Produkte im Inventar den dringendsten Kunden zuordnen
        List<string> carried = new List<string>(ball.Inventory);
        System.Text.StringBuilder sb = new System.Text.StringBuilder("<b>Bestellungen</b>\n");
        foreach (EnemyMovement e in waiting)
        {
            bool ready = carried.Remove(e.Wish.name);
            string time = e.CurrentState == EnemyMovement.State.Arriving ? "kommt" : Mathf.CeilToInt(e.PatienceLeft) + " s";
            string timeColor = e.Patience01 < 0.3f ? "#FF5555" : "#FFFFFF";
            sb.Append(ready ? "<color=#66FF66>[fertig]</color> " : "")
              .Append("<b>").Append(TradeCatalog.Colored(e.Wish.name)).Append("</b> = ")
              .Append(TradeCatalog.RecipeText(e.Wish))
              .Append($"  <color={timeColor}>{time}</color>\n");
        }
        ordersText.text = sb.ToString();
    }

    void BuildEntries()
    {
        foreach (Vector2Int cell in entryCells)
            CreatePad("Entry", CellToBoardLocal(cell.x, cell.y, 0f), new Vector3(0.9f, 0f, 0.9f), entryColor, 0.3f);
    }

    // ================= Spielablauf =================

    void Update()
    {
        if (ball == null) return;

        customers.RemoveAll(c => c == null);
        UpdateSpawning();

        int r = -1, c = -1;
        if (!ball.IsGameOver) BallCell(out r, out c);
        UpdateMachine(r, c);
        UpdateMarket(r, c);
        UpdateCameraTarget();
        UpdateOrdersUi();
    }

    void UpdateSpawning()
    {
        if (ball.IsGameOver || enemy == null || counterCells.Count == 0 || entryCells.Count == 0) return;

        int active = 0;
        foreach (EnemyMovement e in customers)
            if (e.CurrentState != EnemyMovement.State.Leaving) active++;
        int allowed = Mathf.Min(counterCells.Count, 1 + ball.Served / Mathf.Max(1, servedPerExtraCustomer));

        // Ist niemand da, kommt der nächste Kunde schneller
        if (active + pendingCustomers == 0) spawnTimer = Mathf.Min(spawnTimer, 2f);

        spawnTimer -= Time.deltaTime;
        if (spawnTimer > 0f || active + pendingCustomers >= allowed) return;

        int slot = FreeCounterSlot();
        if (slot < 0) return;
        spawnTimer = Mathf.Lerp(spawnIntervalStart, spawnIntervalEnd, Difficulty);
        StartCoroutine(SpawnCustomer(slot));
    }

    float Difficulty => Mathf.Clamp01(ball.Served / (float)Mathf.Max(1, servedForFullDifficulty));

    int FreeCounterSlot()
    {
        List<int> free = new List<int>();
        for (int i = 0; i < counterTaken.Length; i++) if (counterTaken[i] == null) free.Add(i);
        return free.Count > 0 ? free[Random.Range(0, free.Count)] : -1;
    }

    IEnumerator SpawnCustomer(int slot)
    {
        pendingCustomers++;
        counterTaken[slot] = enemy; // reserviert (Platzhalter)

        Vector2Int entry = entryCells[Random.Range(0, entryCells.Count)];
        Vector3 entryLocal = CellToBoardLocal(entry.x, entry.y, 0f);

        // Tür blinkt kurz auf
        GameObject marker = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        marker.name = "EntryWarning";
        Destroy(marker.GetComponent<Collider>());
        Renderer mr = marker.GetComponent<Renderer>();
        mr.material = MakeMaterial(entryColor, 0.8f);
        marker.transform.position = transform.position + entryLocal + Vector3.up * 0.04f;
        for (float t = 0f; t < spawnWarningTime; t += Time.deltaTime)
        {
            float pulse = 0.6f + 0.4f * Mathf.Abs(Mathf.Sin(t * 8f));
            marker.transform.localScale = new Vector3(pulse, 0.02f, pulse);
            // fährt mit dem Brett mit
            marker.transform.position = transform.position + entryLocal + Vector3.up * 0.04f;
            yield return null;
        }
        Destroy(marker);
        pendingCustomers--;

        if (ball == null || ball.IsGameOver) { counterTaken[slot] = null; yield break; }

        Vector3 navPos = playground.StartPosition + entryLocal;
        if (NavMesh.SamplePosition(navPos, out NavMeshHit hit, 2f, NavMesh.AllAreas)) navPos = hit.position;
        Vector2Int cc = counterCells[slot];
        Vector3 counterNav = playground.StartPosition + CellToBoardLocal(cc.x, cc.y, 0f);
        if (NavMesh.SamplePosition(counterNav, out hit, 2f, NavMesh.AllAreas)) counterNav = hit.position;

        TradeCatalog.Product wish = PickWish();
        int reward = Mathf.RoundToInt(TradeCatalog.PartCost(wish) * priceMultiplier);
        float patience = Mathf.Lerp(patienceStart, patienceEnd, Difficulty) + 6f * (wish.parts.Length - 2);

        EnemyMovement clone = Instantiate(enemy, navPos, enemy.transform.rotation);
        clone.name = "Kunde (" + wish.name + ")";
        clone.walkSpeed = customerWalkSpeed;
        clone.gameObject.SetActive(true);
        clone.Teleport(navPos);
        clone.Setup(this, ball, wish, patience, reward, slot, counterNav, navPos);
        StartCoroutine(GrowIn(clone.transform));
        customers.Add(clone);
        counterTaken[slot] = clone;
    }

    // Wunsch aus den freigeschalteten Produkten. Neue Produkte etwas bevorzugen.
    TradeCatalog.Product PickWish()
    {
        List<TradeCatalog.Product> pool = TradeCatalog.Products.FindAll(p => p.unlockAfter <= ball.Served);
        TradeCatalog.Product newest = pool[pool.Count - 1];
        if (pool.Count > 2 && Random.value < 0.3f) return newest;
        return pool[Random.Range(0, pool.Count)];
    }

    IEnumerator GrowIn(Transform t)
    {
        Vector3 full = t.localScale;
        for (float k = 0f; k < 1f; k += Time.deltaTime * 3f)
        {
            if (t == null) yield break;
            t.localScale = full * Mathf.SmoothStep(0f, 1f, k);
            yield return null;
        }
        if (t != null) t.localScale = full;
    }

    // ----- Rückmeldungen der Kunden -----

    public void OnCustomerServed(EnemyMovement customer, int coins)
    {
        Popup($"+{coins} Münzen", new Color(1f, 0.85f, 0.2f), customer.transform.position + Vector3.up * 1.5f);
    }

    public void OnCustomerAngry(EnemyMovement customer)
    {
        Popup("Kunde verärgert!", new Color(1f, 0.3f, 0.25f), customer.transform.position + Vector3.up * 1.5f);
        if (ball != null) ball.LoseReputation();
    }

    public void OnCustomerLeaving(EnemyMovement customer)
    {
        if (customer.CounterSlot >= 0 && customer.CounterSlot < counterTaken.Length
            && counterTaken[customer.CounterSlot] == customer)
            counterTaken[customer.CounterSlot] = null;
    }

    // Wartende Kunden, der ungeduldigste zuerst
    List<EnemyMovement> WaitingCustomers()
    {
        List<EnemyMovement> list = customers.FindAll(c => c != null && c.CurrentState != EnemyMovement.State.Leaving);
        list.Sort((a, b) => a.PatienceLeft.CompareTo(b.PatienceLeft));
        return list;
    }

    // ----- Maschine -----

    void UpdateMachine(int r, int c)
    {
        bool onMachine = r >= 0 && machineGrid[r, c];
        if (!onMachine)
        {
            machineProgress = 0f;
            machineJob = null;
            SetMachineLabel(null);
            return;
        }

        DecideMachineJob(out string job, out bool isAssembly, out string hint);
        if (job != machineJob || isAssembly != machineJobIsAssembly)
        {
            machineJob = job;
            machineJobIsAssembly = isAssembly;
            machineProgress = 0f;
        }
        if (job == null) { SetMachineLabel(hint); return; }

        machineProgress += Time.deltaTime;
        float duration = isAssembly ? assembleTime : partBuildTime;
        if (machineGear != null) machineGear.Rotate(0f, 360f * Time.deltaTime, 0f, Space.Self);

        if (machineProgress >= duration)
        {
            bool done = isAssembly
                ? ball.Assemble(TradeCatalog.FindProduct(job))
                : ball.TryAdd(job);
            if (done)
                Popup((isAssembly ? "" : "+1 ") + TradeCatalog.Colored(job) + (isAssembly ? " fertig!" : ""),
                      Color.white, ball.transform.position + Vector3.up * 1.4f);
            machineProgress = 0f;
            machineJob = null;
            return;
        }

        int percent = Mathf.FloorToInt(100f * machineProgress / duration);
        SetMachineLabel((isAssembly ? "Baut zusammen: " : "Baut Teil: ") + TradeCatalog.Colored(job) + $"  {percent}%");
    }

    // Was soll die Maschine gerade tun?
    //   1. Kann ein gewünschtes Produkt aus dem Inventar gebaut werden -> zusammenbauen
    //   2. Sonst das nächste fehlende Teil für den ungeduldigsten Kunden bauen (langsam)
    void DecideMachineJob(out string job, out bool isAssembly, out string hint)
    {
        job = null; isAssembly = false; hint = "Teile mitbringen";
        List<EnemyMovement> waiting = WaitingCustomers();
        if (waiting.Count == 0) { hint = "Keine Bestellung"; return; }

        // Welche Wünsche sind noch offen (nicht schon als fertiges Produkt dabei)?
        Dictionary<string, int> carried = new Dictionary<string, int>();
        foreach (string item in ball.Inventory)
            carried[item] = carried.TryGetValue(item, out int n) ? n + 1 : 1;
        List<TradeCatalog.Product> open = new List<TradeCatalog.Product>();
        foreach (EnemyMovement e in waiting)
        {
            if (carried.TryGetValue(e.Wish.name, out int n) && n > 0) carried[e.Wish.name] = n - 1;
            else open.Add(e.Wish);
        }
        if (open.Count == 0) { hint = "Alles fertig – ab zu den Kunden!"; return; }

        foreach (TradeCatalog.Product p in open)
            if (ball.HasAll(p.parts)) { job = p.name; isAssembly = true; return; }

        if (ball.IsFull) { hint = "Inventar voll ([Q] wegwerfen)"; return; }

        List<string> missing = ball.Missing(open[0].parts);
        if (missing.Count > 0) job = missing[0];
    }

    void SetMachineLabel(string status)
    {
        if (machineLabel == null) return;
        machineLabel.text = status == null ? "<b>M</b>" : "<b>M</b>\n<size=45%>" + status + "</size>";
    }

    // ----- Markt -----

    void UpdateMarket(int r, int c)
    {
        int stall = r >= 0 ? marketStall[r, c] : -1;
        if (stall < 0) { lastStall = -1; return; }

        if (stall != lastStall)
        {
            lastStall = stall;
            stallTimer = marketRepeatTime;
            TryBuy(stall);
            return;
        }
        stallTimer -= Time.deltaTime;
        if (stallTimer <= 0f)
        {
            stallTimer = marketRepeatTime;
            TryBuy(stall);
        }
    }

    void TryBuy(int stall)
    {
        TradeCatalog.Part part = TradeCatalog.Parts[stall];
        Vector3 at = ball.transform.position + Vector3.up * 1.4f;
        if (ball.IsFull) { Popup("Inventar voll", new Color(1f, 0.5f, 0.4f), at); return; }
        if (!ball.TryPay(part.price)) { Popup("Zu wenig Münzen", new Color(1f, 0.5f, 0.4f), at); return; }
        ball.TryAdd(part.name);
        Popup($"-{part.price}  {TradeCatalog.Colored(part.name)}", Color.white, at);
    }

    // ----- Kamera-Hinweis -----

    void UpdateCameraTarget()
    {
        CurrentTarget = null;
        if (ball.IsGameOver) return;
        foreach (EnemyMovement e in WaitingCustomers())
            if (ball.Has(e.Wish.name)) { CurrentTarget = e.transform; return; }
    }

    // ================= Hilfsfunktionen =================

    // In welcher Zelle steht der Ball gerade? false = außerhalb
    bool BallCell(out int row, out int col)
    {
        Vector3 local = playground.PinPosition - transform.position;
        float half = GridSize * CellSize / 2f;
        col = Mathf.FloorToInt((local.x + half) / CellSize);
        row = Mathf.FloorToInt((half - local.z) / CellSize);
        bool inside = row >= 0 && row < GridSize && col >= 0 && col < GridSize;
        if (!inside) { row = col = -1; }
        return inside;
    }

    static void GetBounds(List<Vector2Int> cells, out int r0, out int r1, out int c0, out int c1)
    {
        r0 = c0 = int.MaxValue; r1 = c1 = int.MinValue;
        foreach (Vector2Int p in cells)
        {
            r0 = Mathf.Min(r0, p.x); r1 = Mathf.Max(r1, p.x);
            c0 = Mathf.Min(c0, p.y); c1 = Mathf.Max(c1, p.y);
        }
    }

    Material MakeMaterial(Color color, float emission)
    {
        Material m = new Material(baseMaterial) { color = color };
        if (m.HasProperty("_BaseColor")) m.SetColor("_BaseColor", color);
        if (emission > 0f && m.HasProperty("_EmissionColor"))
        {
            m.EnableKeyword("_EMISSION");
            m.SetColor("_EmissionColor", color * emission);
        }
        return m;
    }

    // Objekt fährt mit dem Brett mit, ist aber kein Kind von "Ground" -> kein NavMesh-Hindernis
    void PlaceOnBoard(Transform t, Vector3 boardLocal)
    {
        t.position = transform.position + boardLocal;
        t.SetParent(transform, true);
    }

    // Flache, farbige Bodenplatte (der Ball rollt darüber)
    GameObject CreatePad(string name, Vector3 boardLocal, Vector3 size, Color color, float emission, float height = 0.03f)
    {
        GameObject pad = GameObject.CreatePrimitive(PrimitiveType.Cube);
        pad.name = name;
        Destroy(pad.GetComponent<Collider>());
        pad.GetComponent<Renderer>().material = MakeMaterial(color, emission);
        pad.transform.localScale = new Vector3(size.x, height, size.z);
        PlaceOnBoard(pad.transform, boardLocal + Vector3.up * (height / 2f + 0.005f));
        return pad;
    }

    // Schrift, die mit dem Brett mitfährt und schräg zur Kamera zeigt
    TextMeshPro CreateLabel(string name, Vector3 boardLocal, float fontSize, Color color)
    {
        GameObject go = new GameObject(name);
        PlaceOnBoard(go.transform, boardLocal);
        TextMeshPro tmp = go.AddComponent<TextMeshPro>();
        tmp.rectTransform.sizeDelta = new Vector2(4f, 2f);
        tmp.alignment = TextAlignmentOptions.Center;
        tmp.fontSize = fontSize;
        tmp.color = color;
        tmp.richText = true;
        tmp.textWrappingMode = TextWrappingModes.NoWrap;
        go.AddComponent<FaceCamera>();
        return tmp;
    }

    // Aufsteigende, verblassende Meldung an einer Weltposition
    void Popup(string text, Color color, Vector3 worldPos)
    {
        GameObject go = new GameObject("Popup");
        go.transform.position = worldPos;
        go.transform.SetParent(transform, true);
        TextMeshPro tmp = go.AddComponent<TextMeshPro>();
        tmp.rectTransform.sizeDelta = new Vector2(6f, 1f);
        tmp.alignment = TextAlignmentOptions.Center;
        tmp.fontSize = 4f;
        tmp.color = color;
        tmp.richText = true;
        tmp.textWrappingMode = TextWrappingModes.NoWrap;
        tmp.text = "<b>" + text + "</b>";
        go.AddComponent<FaceCamera>();
        StartCoroutine(FloatAway(tmp));
    }

    IEnumerator FloatAway(TextMeshPro tmp)
    {
        const float duration = 1.2f;
        for (float t = 0f; t < duration; t += Time.deltaTime)
        {
            if (tmp == null) yield break;
            tmp.transform.position += Vector3.up * 1.2f * Time.deltaTime;
            tmp.alpha = 1f - t / duration;
            yield return null;
        }
        if (tmp != null) Destroy(tmp.gameObject);
    }

    // ================= Brett, Mauern, NavMesh (wie bisher) =================

    // Steckt ein Kreis (Ball) an dieser Stelle (relativ zum Brett) in einer Mauer?
    public bool IsBlocked(Vector3 boardLocal, float radius)
    {
        float half = GridSize * CellSize / 2f;
        float inner = half - 0.25f; // Innenkante der Außenmauern

        if (Mathf.Abs(boardLocal.x) > inner - radius || Mathf.Abs(boardLocal.z) > inner - radius)
            return true;

        int minCol = Mathf.Max(0, Mathf.FloorToInt((boardLocal.x - radius + half) / CellSize));
        int maxCol = Mathf.Min(GridSize - 1, Mathf.FloorToInt((boardLocal.x + radius + half) / CellSize));
        int minRow = Mathf.Max(0, Mathf.FloorToInt((half - (boardLocal.z + radius)) / CellSize));
        int maxRow = Mathf.Min(GridSize - 1, Mathf.FloorToInt((half - (boardLocal.z - radius)) / CellSize));

        for (int r = minRow; r <= maxRow; r++)
        {
            for (int c = minCol; c <= maxCol; c++)
            {
                if (!wallGrid[r, c]) continue;
                float x0 = -half + c * CellSize, x1 = x0 + CellSize;
                float z1 = half - r * CellSize, z0 = z1 - CellSize;
                float dx = boardLocal.x - Mathf.Clamp(boardLocal.x, x0, x1);
                float dz = boardLocal.z - Mathf.Clamp(boardLocal.z, z0, z1);
                if (dx * dx + dz * dz < radius * radius)
                    return true;
            }
        }
        return false;
    }

    void RebuildNavMesh()
    {
        if (navMeshSurface == null) return;
        // Das NavMesh liegt immer an der Ausgangsposition des Bretts und bewegt
        // sich NICHT mit – die Kunden rechnen den Versatz selbst ein.
        playground.RunWithBoardAtStart(() =>
        {
            navMeshSurface.BuildNavMesh();
            fixedNavMesh.Remove();
            fixedNavMesh = NavMesh.AddNavMeshData(navMeshSurface.navMeshData,
                navMeshSurface.transform.position, navMeshSurface.transform.rotation);
        });
    }

    // Brett-relative Position (Brett in Ausgangslage, ohne Drehung) einer Zelle
    Vector3 CellToBoardLocal(int row, float col, float height)
    {
        float half = GridSize * CellSize / 2f;
        return new Vector3(-half + (col + 0.5f) * CellSize, height, half - (row + 0.5f) * CellSize);
    }

    Vector3 CellToWorld(int row, float col, float height) => transform.position + CellToBoardLocal(row, col, height);

    GameObject SpawnWall(int row, int startCol, int length)
    {
        GameObject wall = GameObject.CreatePrimitive(PrimitiveType.Cube);
        wall.name = "MazeWall";
        if (wallMaterial != null) wall.GetComponent<Renderer>().sharedMaterial = wallMaterial;

        float centerCol = startCol + (length - 1) / 2f;
        wall.transform.position = CellToWorld(row, centerCol, wallHeight / 2f);
        wall.transform.rotation = transform.rotation;
        wall.transform.localScale = new Vector3(length * CellSize, wallHeight, CellSize);

        // Unter "Ground" hängen: bewegt sich mit dem Brett und wird vom NavMesh erfasst
        wall.transform.SetParent(ground != null ? ground : transform, true);
        return wall;
    }

    // Neue Mauer wächst aus dem Boden (nur Optik – die Kollision ist sofort aktiv)
    IEnumerator RiseWall(Transform wall)
    {
        Vector3 fullScale = wall.localScale;
        Vector3 fullPos = wall.localPosition;
        float parentScaleY = wall.parent != null ? wall.parent.lossyScale.y : 1f;
        float fullHeightLocal = wallHeight / Mathf.Max(parentScaleY, 0.0001f);

        for (float t = 0f; t < wallRiseDuration; t += Time.deltaTime)
        {
            if (wall == null) yield break;
            float k = Mathf.SmoothStep(0f, 1f, t / wallRiseDuration);
            wall.localScale = new Vector3(fullScale.x, Mathf.Max(fullScale.y * k, 0.001f), fullScale.z);
            wall.localPosition = fullPos - Vector3.up * (fullHeightLocal * (1f - k) / 2f);
            yield return null;
        }
        if (wall == null) yield break;
        wall.localScale = fullScale;
        wall.localPosition = fullPos;
    }

    // Fester Tisch mit Schachbrettmuster unter dem Spielfeld (bleibt stehen)
    void CreateReferenceTable()
    {
        Vector3 center = playground.StartPosition;
        GameObject table = new GameObject("ReferenceTable");
        table.transform.position = new Vector3(center.x, center.y - tableDepth, center.z);

        Texture2D checker = new Texture2D(2, 2) { filterMode = FilterMode.Point, wrapMode = TextureWrapMode.Repeat };
        checker.SetPixels(new[] { tableColorA, tableColorB, tableColorB, tableColorA });
        checker.Apply();

        GameObject top = GameObject.CreatePrimitive(PrimitiveType.Plane);
        top.name = "TableTop";
        Destroy(top.GetComponent<Collider>());
        top.transform.SetParent(table.transform, false);
        top.transform.localScale = new Vector3(tableSize / 10f, 1f, tableSize / 10f);
        Renderer topRenderer = top.GetComponent<Renderer>();
        Material topMat = new Material(topRenderer.sharedMaterial) { mainTexture = checker, color = Color.white };
        float tiles = tableSize / (tileSize * 2f);
        topMat.mainTextureScale = new Vector2(tiles, tiles);
        topRenderer.sharedMaterial = topMat;

        Material frameMat = new Material(topRenderer.sharedMaterial) { mainTexture = null, color = homeFrameColor };
        float half = GridSize * CellSize / 2f + 0.5f;
        float thickness = 0.3f;
        Vector3[] positions =
        {
            new Vector3(0f, 0.02f, half), new Vector3(0f, 0.02f, -half),
            new Vector3(half, 0.02f, 0f), new Vector3(-half, 0.02f, 0f)
        };
        Vector3[] scales =
        {
            new Vector3(2f * half + thickness, 0.02f, thickness), new Vector3(2f * half + thickness, 0.02f, thickness),
            new Vector3(thickness, 0.02f, 2f * half + thickness), new Vector3(thickness, 0.02f, 2f * half + thickness)
        };
        for (int i = 0; i < 4; i++)
        {
            GameObject strip = GameObject.CreatePrimitive(PrimitiveType.Cube);
            strip.name = "HomeFrame";
            Destroy(strip.GetComponent<Collider>());
            strip.transform.SetParent(table.transform, false);
            strip.transform.localPosition = positions[i];
            strip.transform.localScale = scales[i];
            strip.GetComponent<Renderer>().sharedMaterial = frameMat;
        }
    }
}
