using System.Collections.Generic;
using Unity.AI.Navigation;
using UnityEngine;
using UnityEngine.AI;

// Eine Zelle des feinen Rasters. Auf einer Zelle liegt entweder nichts,
// eine Straße oder ein Gebäude. Zellen innerhalb eines großen Feldes gehören zu diesem Feld.
public class Cell
{
    public int x, y;
    public Vector3 position;
    public CityField field;      // null = Weg zwischen den Feldern / Rand
    public bool owned;           // gehört zur Stadt (Feld gekauft bzw. Weg daneben)
    public bool road;
    public Building building;
    public GameObject roadObject;
    public bool IsFree => !road && building == null;
}

// Erzeugt das Spielbrett: grauer Tisch wie im alten Spiel, darauf 2x2 große Felder,
// die der Spieler einfärbt (Wohnen, Industrie, ...). Rundherum liegt Bauland, um das
// die Stadt gegen Geld erweitert werden kann. Verwaltet Straßen, Gebäude,
// Bewohner und das NavMesh.
public class CityGrid : MonoBehaviour
{
    public const int RoadArea = 3; // NavMesh-Bereich für Straßen (dort laufen Bewohner lieber)

    [Header("Felder")]
    [Tooltip("Felder pro Seite, mit denen das Spiel beginnt (2 = 4 Felder)")]
    public int startFieldsPerSide = 2;
    [Tooltip("Felder pro Seite, wenn die Stadt voll erweitert ist")]
    public int maxFieldsPerSide = 4;
    [Tooltip("Zellen pro Feld und Seite (pro Zelle passt ein Haus)")]
    public int cellsPerField = 8;
    public float cellSize = 1.5f;

    [Header("Aussehen (wie im alten Spiel)")]
    public Color fieldColor = new Color(0.51f, 0.51f, 0.51f);
    [Tooltip("Bauland, das noch nicht zur Stadt gehört")]
    public Color unownedColor = new Color(0.27f, 0.28f, 0.31f);
    public Color boardColor = new Color(0.38f, 0.39f, 0.42f);
    public Color roadColor = new Color(0.2f, 0.2f, 0.22f);
    public Color tableColorA = new Color(0.30f, 0.32f, 0.36f);
    public Color tableColorB = new Color(0.40f, 0.42f, 0.47f);
    public float tableSize = 90f;

    [Header("Bewohner")]
    public float residentSpeed = 2.5f;
    [Tooltip("Geschwindigkeitsfaktor auf Straßen")]
    public float roadSpeedBoost = 1.8f;

    public int Size { get; private set; }
    public IReadOnlyList<CityField> Fields => fields;
    public IReadOnlyList<CityField> OwnedFields => ownedFields;
    public int Expansions { get; private set; }
    public event System.Action LandChanged;
    public IReadOnlyList<Resident> Residents => residents;
    public Economy Economy => economy;

    private Cell[,] cells;
    private readonly List<CityField> fields = new List<CityField>();
    private readonly List<CityField> ownedFields = new List<CityField>();
    private CityField[,] fieldGrid;
    private Transform board;
    private int nextFieldNumber = 1;
    private readonly List<Resident> residents = new List<Resident>();
    private Economy economy;
    private NavMeshSurface surface;
    private Transform navRoot, buildingParent, residentParent;
    private bool navDirty;

    void Awake()
    {
        economy = GetComponent<Economy>();
        // Gehen neben der Straße kostet mehr -> Bewohner nutzen Straßen, wenn es sie gibt
        NavMesh.SetAreaCost(0, 2.5f);
        NavMesh.SetAreaCost(RoadArea, 1f);

        // 1 Zelle Rand + Felder + 1 Zelle Weg zwischen je zwei Feldern + 1 Zelle Rand
        Size = maxFieldsPerSide * cellsPerField + (maxFieldsPerSide - 1) + 2;

        CreateTable();
        CreateBoard();
        CreateCellsAndFields();

        buildingParent = new GameObject("Gebaeude").transform;
        buildingParent.SetParent(transform, false);
        residentParent = new GameObject("Bewohner").transform;
        residentParent.SetParent(transform, false);

        // Startfelder in der Mitte gehören der Stadt (oben links = Feld 1)
        int start = (maxFieldsPerSide - startFieldsPerSide) / 2;
        for (int fy = start + startFieldsPerSide - 1; fy >= start; fy--)
        for (int fx = start; fx < start + startFieldsPerSide; fx++)
            Own(fieldGrid[fx, fy]);
        RefreshLand();

        // Vorgebautes Straßenkreuz zwischen den Startfeldern
        GetOwnedCellBounds(out int minX, out int minY, out int maxX, out int maxY);
        for (int x = minX; x <= maxX; x++)
        for (int y = minY; y <= maxY; y++)
            if (cells[x, y].field == null) SetRoad(cells[x, y], true);
        surface.BuildNavMesh();
        navDirty = false;
    }

    void LateUpdate()
    {
        // Mehrere Straßen in einem Frame (Ziehen mit der Maus) -> nur einmal neu backen
        if (navDirty)
        {
            surface.BuildNavMesh();
            navDirty = false;
        }
    }

    // ---- Aufbau ----

    void CreateTable()
    {
        // Schachbrett-Tisch wie im Roll-a-Ball-Level
        Texture2D checker = new Texture2D(2, 2) { filterMode = FilterMode.Point, wrapMode = TextureWrapMode.Repeat };
        checker.SetPixels(new[] { tableColorA, tableColorB, tableColorB, tableColorA });
        checker.Apply();

        GameObject top = GameObject.CreatePrimitive(PrimitiveType.Plane);
        top.name = "Tisch";
        Destroy(top.GetComponent<Collider>());
        top.transform.SetParent(transform, false);
        top.transform.localPosition = new Vector3(0f, -0.6f, 0f);
        top.transform.localScale = new Vector3(tableSize / 10f, 1f, tableSize / 10f);
        Material mat = new Material(CityMaterials.Get(Color.white)) { mainTexture = checker };
        mat.SetTexture("_BaseMap", checker);
        float tiles = tableSize / (cellSize * 2f);
        mat.mainTextureScale = new Vector2(tiles, tiles);
        mat.SetTextureScale("_BaseMap", new Vector2(tiles, tiles));
        top.GetComponent<Renderer>().sharedMaterial = mat;
    }

    void CreateBoard()
    {
        // Alles, was das NavMesh beeinflusst, liegt unter navRoot
        navRoot = new GameObject("NavMesh").transform;
        navRoot.SetParent(transform, false);
        surface = navRoot.gameObject.AddComponent<NavMeshSurface>();
        surface.collectObjects = CollectObjects.Children;
        surface.useGeometry = NavMeshCollectGeometry.PhysicsColliders;

        // Größe und Lage setzt RefreshLand (das Brett wächst mit der Stadt)
        GameObject go = GameObject.CreatePrimitive(PrimitiveType.Cube);
        go.name = "Brett";
        go.transform.SetParent(navRoot, false);
        go.GetComponent<Renderer>().sharedMaterial = CityMaterials.Get(boardColor);
        board = go.transform;
    }

    void CreateCellsAndFields()
    {
        cells = new Cell[Size, Size];
        for (int x = 0; x < Size; x++)
        for (int y = 0; y < Size; y++)
            cells[x, y] = new Cell { x = x, y = y, position = CellToWorld(x, y) };

        Transform parent = new GameObject("Felder").transform;
        parent.SetParent(transform, false);
        fieldGrid = new CityField[maxFieldsPerSide, maxFieldsPerSide];
        for (int fy = 0; fy < maxFieldsPerSide; fy++)
        for (int fx = 0; fx < maxFieldsPerSide; fx++)
        {
            int startX = 1 + fx * (cellsPerField + 1);
            int startY = 1 + fy * (cellsPerField + 1);
            var fieldCells = new List<Cell>();
            for (int x = startX; x < startX + cellsPerField; x++)
            for (int y = startY; y < startY + cellsPerField; y++)
                fieldCells.Add(cells[x, y]);

            GameObject go = new GameObject($"Bauland {fx},{fy}");
            go.transform.SetParent(parent, false);
            Vector3 a = CellToWorld(startX, startY), b = CellToWorld(startX + cellsPerField - 1, startY + cellsPerField - 1);
            go.transform.position = (a + b) * 0.5f;
            CityField field = go.AddComponent<CityField>();
            field.Init(this, fx, fy, fieldCells, cellsPerField * cellSize, fieldColor, unownedColor);
            foreach (Cell c in fieldCells) c.field = field;
            fields.Add(field);
            fieldGrid[fx, fy] = field;
        }
    }

    // ---- Koordinaten ----

    public Vector3 CellToWorld(int x, int y)
    {
        float half = (Size - 1) * cellSize * 0.5f;
        return transform.position + new Vector3(x * cellSize - half, 0f, y * cellSize - half);
    }

    public Cell GetCell(int x, int y)
    {
        if (x < 0 || y < 0 || x >= Size || y >= Size) return null;
        return cells[x, y];
    }

    public Cell GetCellAt(Vector3 world)
    {
        Vector3 origin = CellToWorld(0, 0);
        return GetCell(Mathf.RoundToInt((world.x - origin.x) / cellSize), Mathf.RoundToInt((world.z - origin.z) / cellSize));
    }

    public bool IsNextToRoad(Cell c)
    {
        return IsRoad(c.x + 1, c.y) || IsRoad(c.x - 1, c.y) || IsRoad(c.x, c.y + 1) || IsRoad(c.x, c.y - 1);
    }

    bool IsRoad(int x, int y)
    {
        Cell c = GetCell(x, y);
        return c != null && c.road;
    }

    // ---- Straßen ----

    public bool CanBuildRoad(Cell c) => c != null && c.owned && c.IsFree;

    public void SetRoad(Cell c, bool road)
    {
        if (c == null || c.road == road) return;
        if (road && c.building != null) return;
        c.road = road;

        if (road)
        {
            GameObject go = GameObject.CreatePrimitive(PrimitiveType.Cube);
            go.name = $"Strasse {c.x},{c.y}";
            Destroy(go.GetComponent<Collider>());
            go.transform.SetParent(navRoot, true);
            go.transform.position = c.position + Vector3.up * 0.03f;
            go.transform.localScale = new Vector3(cellSize, 0.04f, cellSize);
            go.GetComponent<Renderer>().sharedMaterial = CityMaterials.Get(roadColor);

            // Bereich im NavMesh als "Straße" markieren
            NavMeshModifierVolume volume = go.AddComponent<NavMeshModifierVolume>();
            volume.size = new Vector3(1f, 20f, 1f); // in lokalen Einheiten (Würfel ist skaliert)
            volume.center = Vector3.zero;
            volume.area = RoadArea;
            c.roadObject = go;
        }
        else if (c.roadObject != null)
        {
            Destroy(c.roadObject);
            c.roadObject = null;
        }
        navDirty = true;
    }

    // ---- Erweiterung ----

    public float ExpansionPrice =>
        economy.rules.expansionBaseCost * Mathf.Pow(economy.rules.expansionCostGrowth, Expansions);

    // Erweitern geht nur an Felder, die an die Stadt angrenzen
    public bool CanExpand(CityField f)
    {
        if (f == null || f.Owned) return false;
        return IsOwned(f.GridX + 1, f.GridY) || IsOwned(f.GridX - 1, f.GridY)
            || IsOwned(f.GridX, f.GridY + 1) || IsOwned(f.GridX, f.GridY - 1);
    }

    bool IsOwned(int fx, int fy)
    {
        if (fx < 0 || fy < 0 || fx >= maxFieldsPerSide || fy >= maxFieldsPerSide) return false;
        return fieldGrid[fx, fy].Owned;
    }

    // Gibt eine Fehlermeldung zurück oder null bei Erfolg
    public string TryExpand(CityField f)
    {
        if (f == null || f.Owned) return null;
        if (!CanExpand(f)) return "Die Stadt kann nur an angrenzende Felder erweitert werden";
        float price = ExpansionPrice;
        if (!economy.TrySpend(price, Ledger.Erweiterung)) return $"Nicht genug Geld: Erweiterung kostet {price:0}";
        Expansions++;
        Own(f);
        RefreshLand();
        return null;
    }

    void Own(CityField f)
    {
        f.SetOwned(nextFieldNumber++);
        f.name = $"Feld {f.Number}";
        ownedFields.Add(f);
    }

    void GetOwnedCellBounds(out int minX, out int minY, out int maxX, out int maxY)
    {
        minX = minY = int.MaxValue;
        maxX = maxY = int.MinValue;
        foreach (CityField f in ownedFields)
            foreach (Cell c in f.Cells)
            {
                minX = Mathf.Min(minX, c.x); maxX = Mathf.Max(maxX, c.x);
                minY = Mathf.Min(minY, c.y); maxY = Mathf.Max(maxY, c.y);
            }
    }

    // Brett, Bauland-Anzeige und Besitz der Wegzellen an die Stadtgrenzen anpassen
    void RefreshLand()
    {
        foreach (CityField f in fields)
        {
            if (f.Owned) f.ShowOwned();
            else if (CanExpand(f)) f.ShowForSale(ExpansionPrice);
            else f.ShowHidden();
        }

        // Wegzellen gehören zur Stadt, wenn sie an ein eigenes Feld grenzen
        for (int x = 0; x < Size; x++)
        for (int y = 0; y < Size; y++)
        {
            Cell c = cells[x, y];
            if (c.field != null) { c.owned = c.field.Owned; continue; }
            c.owned = false;
            for (int dx = -1; dx <= 1 && !c.owned; dx++)
            for (int dy = -1; dy <= 1 && !c.owned; dy++)
            {
                Cell n = GetCell(x + dx, y + dy);
                if (n != null && n.field != null && n.field.Owned) c.owned = true;
            }
        }

        // Brett: umfasst alle eigenen Felder plus eine Zelle Rand
        GetOwnedCellBounds(out int minX, out int minY, out int maxX, out int maxY);
        minX = Mathf.Max(0, minX - 1); minY = Mathf.Max(0, minY - 1);
        maxX = Mathf.Min(Size - 1, maxX + 1); maxY = Mathf.Min(Size - 1, maxY + 1);
        Vector3 center = (CellToWorld(minX, minY) + CellToWorld(maxX, maxY)) * 0.5f;
        board.position = new Vector3(center.x, transform.position.y - 0.3f, center.z);
        board.localScale = new Vector3((maxX - minX + 1) * cellSize, 0.6f, (maxY - minY + 1) * cellSize);

        navDirty = true;
        LandChanged?.Invoke();
    }

    // ---- Gebäude ----

    // Freie Zelle im Feld neben einer Straße (ohne Straße keine Bebauung)
    public Cell FindFreeLot(CityField field)
    {
        var options = new List<Cell>();
        foreach (Cell c in field.Cells)
            if (c.IsFree && IsNextToRoad(c)) options.Add(c);
        return options.Count == 0 ? null : options[Random.Range(0, options.Count)];
    }

    public Building CreateBuilding(CityField field, Cell cell, int level)
    {
        GameObject go = new GameObject($"{field.Zone.displayName} {cell.x},{cell.y}");
        go.transform.SetParent(buildingParent, false);
        go.transform.position = cell.position;
        Building b = go.AddComponent<Building>();
        cell.building = b;
        field.Buildings.Add(b);
        b.Init(this, field, cell, level);
        return b;
    }

    public Building FindNearest(ZoneKind kind, Vector3 from, System.Predicate<Building> condition = null)
    {
        Building best = null;
        float bestDist = float.MaxValue;
        foreach (CityField f in fields)
        {
            if (!f.Is(kind)) continue;
            foreach (Building b in f.Buildings)
            {
                if (b == null || b.Level <= 0) continue;
                if (condition != null && !condition(b)) continue;
                float d = (b.transform.position - from).sqrMagnitude;
                if (d < bestDist) { bestDist = d; best = b; }
            }
        }
        return best;
    }

    public bool HasInstitutionNear(Vector3 position)
    {
        if (economy != null && economy.Treasury <= 0f) return false; // Kasse leer -> Institutionen wirken nicht
        foreach (CityField f in fields)
        {
            if (!f.Is(ZoneKind.Institution)) continue;
            float r = f.Zone.influenceRadius * cellSize;
            foreach (Building b in f.Buildings)
                if (b != null && (b.transform.position - position).sqrMagnitude <= r * r) return true;
        }
        return false;
    }

    // ---- Statistik ----

    public float TotalUpkeep()
    {
        float sum = 0f;
        foreach (CityField f in fields)
            if (f.Zone != null && f.Buildings.Count > 0) sum += f.Zone.upkeepPerTick;
        return sum;
    }

    public int TotalJobs()
    {
        int n = 0;
        foreach (CityField f in fields)
            if (f.Is(ZoneKind.Industrie))
                foreach (Building b in f.Buildings) n += b.Capacity;
        return n;
    }

    public int FreeJobs()
    {
        int free = 0;
        foreach (CityField f in fields)
            if (f.Is(ZoneKind.Industrie))
                foreach (Building b in f.Buildings) free += b.Capacity - b.Workers;
        return free;
    }

    public int Unemployed()
    {
        int n = 0;
        foreach (Resident r in residents) if (!r.HasJob) n++;
        return n;
    }

    public float AverageSatisfaction()
    {
        if (residents.Count == 0) return 0f;
        float sum = 0f;
        foreach (Resident r in residents) sum += r.Satisfaction;
        return sum / residents.Count;
    }

    // ---- Wachstum (vom Economy-Tick aufgerufen) ----

    public void UpdateGrowth(EconomySystem rules)
    {
        int freeJobs = FreeJobs();
        // Zuzug: wegen freier Jobs, oder weil die Stadt attraktiv ist (zufrieden, keine Arbeitslosen).
        // Letzteres erzeugt Arbeitslosigkeit, die wiederum Firmen zum Investieren bringt.
        bool attractive = residents.Count > 0 && Unemployed() == 0 && AverageSatisfaction() >= rules.growthThreshold;

        foreach (CityField f in fields)
        {
            if (f.Zone == null) continue;

            switch (f.Zone.kind)
            {
                case ZoneKind.Wohnen:
                    // Bestehende Häuser wachsen oder schrumpfen je nach Zufriedenheit
                    foreach (Building h in f.Buildings.ToArray())
                    {
                        if (Random.value > rules.growthChancePerTick) continue;
                        float sat = h.AverageSatisfaction();
                        if (sat >= rules.growthThreshold && (freeJobs > 0 || attractive) && h.Level < f.Zone.maxLevel)
                        {
                            h.SetLevel(h.Level + 1);
                            freeJobs -= f.Zone.capacityPerLevel;
                        }
                        else if (sat <= rules.shrinkThreshold)
                        {
                            if (h.Level <= 1) h.Remove(); // verlassen
                            else h.SetLevel(h.Level - 1);
                        }
                    }
                    // Neues Haus: wenn es Arbeit gibt, die Stadt attraktiv oder noch leer ist
                    float chance = freeJobs > 0 || residents.Count == 0 ? rules.growthChancePerTick * 2f
                                 : attractive ? rules.growthChancePerTick * 0.5f : 0f;
                    if (Random.value < chance)
                    {
                        Cell lot = FindFreeLot(f);
                        if (lot != null)
                        {
                            CreateBuilding(f, lot, 1);
                            freeJobs -= f.Zone.capacityPerLevel;
                        }
                    }
                    break;

                case ZoneKind.Industrie:
                    // Erste Werkstatt sofort; weitere investiert die Firma selbst,
                    // wenn alle Jobs besetzt sind, es Arbeitslose gibt und Kapital da ist.
                    if (f.Buildings.Count == 0)
                        TryBuild(f);
                    else if (freeJobs <= 0 && Unemployed() > 0 && Random.value < rules.growthChancePerTick
                             && economy.TrySpendFirm(rules.investCost, Ledger.Investitionen))
                    {
                        if (TryBuild(f)) freeJobs += f.Zone.capacityPerLevel;
                        else economy.RefundFirm(rules.investCost, Ledger.Investitionen);
                    }
                    break;

                default:
                    // Agora und Institution: ein Gebäude pro Feld
                    if (f.Buildings.Count == 0) TryBuild(f);
                    break;
            }
        }
    }

    bool TryBuild(CityField f)
    {
        Cell lot = FindFreeLot(f);
        if (lot == null) return false;
        CreateBuilding(f, lot, 1);
        return true;
    }

    // ---- Bewohner ----

    public Resident SpawnResident(Building home)
    {
        GameObject go = GameObject.CreatePrimitive(PrimitiveType.Capsule);
        go.name = "Bewohner";
        Destroy(go.GetComponent<Collider>());
        go.transform.SetParent(residentParent, true);
        go.transform.localScale = new Vector3(0.3f, 0.3f, 0.3f);
        go.transform.position = home.transform.position + Vector3.up * 0.3f;

        Resident r = go.AddComponent<Resident>();
        r.Init(this, home, residentSpeed, roadSpeedBoost, economy != null ? economy.rules.startResidentMoney : 10f);
        residents.Add(r);
        return r;
    }

    public void RemoveResident(Resident r) => residents.Remove(r);

    public void FireAllWorkers(Building workplace)
    {
        foreach (Resident r in residents)
            if (r.Job == workplace) r.LoseJob();
    }
}
