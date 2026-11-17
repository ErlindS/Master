using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.InputSystem;

// Der Spieler gibt das System vor: Er färbt Felder ein (Feldtyp), baut Straßen
// und stellt die Regeln ein. Handeln tun dann die Bewohner von selbst.
// Welches Werkzeug aktiv ist, wählt das Menü (CityUI).
public class BuildTool : MonoBehaviour
{
    public enum Mode { None, Zone, ClearZone, Road, RemoveRoad, Expand }

    [Tooltip("Feldtypen in der Reihenfolge, in der sie im Menü erscheinen")]
    public ZoneDefinition[] zones;

    public Mode CurrentMode { get; private set; }
    public ZoneDefinition SelectedZone { get; private set; }
    public CityField HoveredField { get; private set; }
    public Cell HoveredCell { get; private set; }
    public string Message { get; private set; } = "";
    public float MessageUntil { get; private set; }

    public event System.Action Changed;

    private CityGrid grid;
    private Economy economy;
    private Camera cam;
    private Transform cursor;
    private Renderer cursorRenderer;
    private Cell lastRoadCell;

    void Start()
    {
        grid = GetComponent<CityGrid>();
        economy = GetComponent<Economy>();
        cam = Camera.main;

        GameObject c = GameObject.CreatePrimitive(PrimitiveType.Cube);
        c.name = "Cursor";
        Destroy(c.GetComponent<Collider>());
        cursor = c.transform;
        cursorRenderer = c.GetComponent<Renderer>();
        cursor.gameObject.SetActive(false);
    }

    // ---- Vom Menü aufgerufen ----

    public void SelectZone(ZoneDefinition zone) { SelectedZone = zone; SetMode(Mode.Zone); }
    public void SelectClearZone() => SetMode(Mode.ClearZone);
    public void SelectRoad() => SetMode(Mode.Road);
    public void SelectRemoveRoad() => SetMode(Mode.RemoveRoad);
    public void SelectExpand() => SetMode(Mode.Expand);
    public void Cancel() => SetMode(Mode.None);

    void SetMode(Mode mode)
    {
        CurrentMode = mode;
        Changed?.Invoke();
    }

    public void ShowMessage(string text)
    {
        Message = text;
        MessageUntil = Time.unscaledTime + 3f;
    }

    // ---- Eingabe ----

    void Update()
    {
        Keyboard kb = Keyboard.current;
        if (kb != null)
        {
            if (kb.escapeKey.wasPressedThisFrame) Cancel();
            if (kb.spaceKey.wasPressedThisFrame) Time.timeScale = Time.timeScale > 0f ? 0f : 1f;
        }

        Mouse mouse = Mouse.current;
        if (mouse == null || cam == null) return;
        if (mouse.rightButton.wasPressedThisFrame) Cancel();

        bool overUI = EventSystem.current != null && EventSystem.current.IsPointerOverGameObject();
        HoveredCell = null;
        HoveredField = null;
        if (!overUI)
        {
            Ray ray = cam.ScreenPointToRay(mouse.position.ReadValue());
            if (new Plane(Vector3.up, grid.transform.position).Raycast(ray, out float dist))
            {
                HoveredCell = grid.GetCellAt(ray.GetPoint(dist));
                HoveredField = HoveredCell != null ? HoveredCell.field : null;
            }
        }

        UpdateCursor();
        if (overUI) return;

        switch (CurrentMode)
        {
            case Mode.Zone:
            case Mode.ClearZone:
                if (mouse.leftButton.wasPressedThisFrame && HoveredField != null) ApplyZone(HoveredField);
                break;

            case Mode.Expand:
                if (mouse.leftButton.wasPressedThisFrame && HoveredField != null && !HoveredField.Owned)
                {
                    string error = grid.TryExpand(HoveredField);
                    if (error != null) ShowMessage(error);
                }
                break;

            case Mode.Road:
            case Mode.RemoveRoad:
                // Klicken und ziehen
                if (!mouse.leftButton.isPressed) { lastRoadCell = null; break; }
                if (HoveredCell != null && HoveredCell != lastRoadCell)
                {
                    lastRoadCell = HoveredCell;
                    ApplyRoad(HoveredCell);
                }
                break;
        }
    }

    void ApplyZone(CityField field)
    {
        if (!field.Owned)
        {
            ShowMessage("Dieses Bauland gehört noch nicht zur Stadt: erst \"Stadt erweitern\"");
            return;
        }
        if (CurrentMode == Mode.ClearZone)
        {
            if (field.Zone != null) field.SetZone(null);
            return;
        }
        if (SelectedZone == null || field.Zone == SelectedZone) return;
        if (!economy.TrySpend(SelectedZone.buildCost, Ledger.FelderAusweisen))
        {
            ShowMessage($"Nicht genug Geld: {SelectedZone.displayName} kostet {SelectedZone.buildCost}");
            return;
        }
        field.SetZone(SelectedZone);
        if (!field.HasRoadAccess()) ShowMessage("Dieses Feld braucht eine Straße, sonst wird nichts gebaut");
    }

    void ApplyRoad(Cell cell)
    {
        if (CurrentMode == Mode.RemoveRoad)
        {
            if (cell.road) grid.SetRoad(cell, false);
            return;
        }
        if (!grid.CanBuildRoad(cell)) return;
        if (!economy.TrySpend(economy.rules.roadCost, Ledger.Strassenbau))
        {
            ShowMessage("Nicht genug Geld für die Straße");
            return;
        }
        grid.SetRoad(cell, true);
    }

    void UpdateCursor()
    {
        bool zoneMode = CurrentMode == Mode.Zone || CurrentMode == Mode.ClearZone || CurrentMode == Mode.Expand;
        bool roadMode = CurrentMode == Mode.Road || CurrentMode == Mode.RemoveRoad;
        bool show = (zoneMode && HoveredField != null && HoveredField.gameObject.activeSelf) || (roadMode && HoveredCell != null);
        cursor.gameObject.SetActive(show);
        if (!show) return;

        Color c;
        if (zoneMode)
        {
            cursor.position = HoveredField.transform.position + Vector3.up * 0.06f;
            float s = HoveredField.WorldSize;
            cursor.localScale = new Vector3(s, 0.02f, s);
            if (CurrentMode == Mode.Expand)
                c = grid.CanExpand(HoveredField) ? new Color(1f, 0.75f, 0.2f) : new Color(0.9f, 0.25f, 0.2f);
            else if (!HoveredField.Owned)
                c = new Color(0.9f, 0.25f, 0.2f);
            else
                c = CurrentMode == Mode.ClearZone ? new Color(0.9f, 0.3f, 0.3f) : SelectedZone.groundColor;
            c = Color.Lerp(c, Color.white, 0.35f);
        }
        else
        {
            cursor.position = HoveredCell.position + Vector3.up * 0.07f;
            cursor.localScale = new Vector3(grid.cellSize, 0.03f, grid.cellSize);
            bool ok = CurrentMode == Mode.RemoveRoad ? HoveredCell.road : grid.CanBuildRoad(HoveredCell);
            c = ok ? new Color(1f, 0.75f, 0.2f) : new Color(0.9f, 0.25f, 0.2f);
        }
        cursorRenderer.sharedMaterial = CityMaterials.Get(c);
    }
}
