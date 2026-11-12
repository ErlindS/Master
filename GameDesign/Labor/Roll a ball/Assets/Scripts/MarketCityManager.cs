using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.InputSystem;

public enum GalleriaShopType
{
    Boutique,
    Cafe,
    Juwelier
}

public class MarketCityManager : MonoBehaviour
{
    [Header("Wirtschaft & Ressourcen")]
    public int lire = 350;
    public int marmor = 60;
    public int seide = 25;
    public int luxuswaren = 0;
    public int tag = 1;
    public int flaneureAnzahl = 16;

    [Header("Tageszyklus")]
    public float tagesDauer = 20f;
    private float tagesTimer;

    [Header("Bauen in der Galleria")]
    public GalleriaShopType selectedShop = GalleriaShopType.Boutique;
    private Vector2Int cursor = new Vector2Int(0, 0);
    private readonly Dictionary<Vector2Int, GameObject> builtShopObjects = new Dictionary<Vector2Int, GameObject>();
    private readonly Dictionary<Vector2Int, GalleriaShopType> builtShopTypes = new Dictionary<Vector2Int, GalleriaShopType>();
    private readonly List<GameObject> citizens = new List<GameObject>();

    private GameObject cursorMarker;
    private TextMeshProUGUI hudText;
    private Light sunLight;

    private Material marbleMat;
    private Material ironMat;
    private Material storefrontMat;
    private Material goldMat;

    void Start()
    {
        sunLight = FindAnyObjectByType<Light>();
        InitMaterials();
        BuildGalleriaHall();
        CreateCursorMarker();
        CreateHUD();

        // 4 Start-Boutiquen in den 4 Quadranten des Kreuzgangs
        TryBuildAt(new Vector2Int(-1, 1), GalleriaShopType.Boutique, true);
        TryBuildAt(new Vector2Int(1, 1), GalleriaShopType.Cafe, true);
        TryBuildAt(new Vector2Int(-1, -1), GalleriaShopType.Cafe, true);
        TryBuildAt(new Vector2Int(1, -1), GalleriaShopType.Juwelier, true);

        SpawnFlaneure();
    }

    void InitMaterials()
    {
        Shader litShader = Shader.Find("Universal Render Pipeline/Lit") ?? Shader.Find("Standard");

        marbleMat = new Material(litShader);
        marbleMat.color = new Color(0.93f, 0.90f, 0.83f); // Edler weiß-cremefarbener Carrara-Marmor
        if (marbleMat.HasProperty("_Smoothness")) marbleMat.SetFloat("_Smoothness", 0.65f);

        ironMat = new Material(litShader);
        ironMat.color = new Color(0.18f, 0.22f, 0.25f); // Gusseisen dunkelgrau/schwarz
        if (ironMat.HasProperty("_Metallic")) ironMat.SetFloat("_Metallic", 0.8f);

        storefrontMat = new Material(litShader);
        storefrontMat.color = new Color(0.15f, 0.14f, 0.12f); // Dunkles Nussbaum-Holz & Schaufenster
        if (storefrontMat.HasProperty("_Smoothness")) storefrontMat.SetFloat("_Smoothness", 0.75f);

        goldMat = new Material(litShader);
        goldMat.color = new Color(0.95f, 0.78f, 0.25f); // Goldverzierungen
        if (goldMat.HasProperty("_Metallic")) goldMat.SetFloat("_Metallic", 0.9f);
    }

    void BuildGalleriaHall()
    {
        // 1. Galleria Boden mit Mosaikfeldern
        Transform ground = transform.Find("Ground");
        if (ground != null)
        {
            ground.localScale = new Vector3(4.5f, 1f, 4.5f);
            Renderer r = ground.GetComponent<Renderer>();
            if (r != null) r.sharedMaterial = marbleMat;
        }

        // Zentrales Oktogon-Mosaik
        GameObject octo = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        octo.name = "MilanoCenterMosaic";
        Destroy(octo.GetComponent<Collider>());
        octo.transform.position = new Vector3(0f, 0.04f, 0f);
        octo.transform.localScale = new Vector3(4.5f, 0.02f, 4.5f);
        Material octoMat = new Material(marbleMat);
        octoMat.color = new Color(0.25f, 0.42f, 0.58f); // Blau-Marmor
        octo.GetComponent<Renderer>().sharedMaterial = octoMat;

        // 2. Gusseisernes Tonnengewölbe (Nord-Süd & Ost-West Kreuzgang)
        GameObject vaultNS = MilanoMeshGenerator.CreateGlassBarrelVault("Vault_NS", 7.5f, 24f, 5.2f, ironMat, marbleMat);
        vaultNS.transform.SetParent(transform, false);

        GameObject vaultEW = MilanoMeshGenerator.CreateGlassBarrelVault("Vault_EW", 7.5f, 24f, 5.2f, ironMat, marbleMat);
        vaultEW.transform.SetParent(transform, false);
        vaultEW.transform.localRotation = Quaternion.Euler(0f, 90f, 0f);

        // Zentrale Glaskuppel über der Kreuzung
        GameObject domeCenter = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        domeCenter.name = "CentralGlassDome";
        Destroy(domeCenter.GetComponent<Collider>());
        domeCenter.transform.SetParent(transform, false);
        domeCenter.transform.localPosition = new Vector3(0f, 8.8f, 0f);
        domeCenter.transform.localScale = new Vector3(10f, 5.5f, 10f);
        Material domeMat = new Material(marbleMat);
        domeMat.color = new Color(0.45f, 0.70f, 0.85f, 0.35f);
        domeCenter.GetComponent<Renderer>().sharedMaterial = domeMat;
    }

    void CreateCursorMarker()
    {
        cursorMarker = GameObject.CreatePrimitive(PrimitiveType.Cube);
        cursorMarker.name = "MilanoCursor";
        Destroy(cursorMarker.GetComponent<Collider>());
        cursorMarker.transform.localScale = new Vector3(3.2f, 0.06f, 3.2f);
        cursorMarker.GetComponent<Renderer>().sharedMaterial = goldMat;
        UpdateCursorPosition();
    }

    void CreateHUD()
    {
        Canvas canvas = FindAnyObjectByType<Canvas>();
        if (canvas == null)
        {
            GameObject cObj = new GameObject("GalleriaCanvas");
            canvas = cObj.AddComponent<Canvas>();
            canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        }

        GameObject hudObj = new GameObject("MilanoHUD");
        hudObj.transform.SetParent(canvas.transform, false);
        RectTransform rt = hudObj.AddComponent<RectTransform>();
        rt.anchorMin = new Vector2(0f, 1f);
        rt.anchorMax = new Vector2(1f, 1f);
        rt.pivot = new Vector2(0.5f, 1f);
        rt.anchoredPosition = new Vector2(0f, -14f);
        rt.sizeDelta = new Vector2(0f, 44f);

        hudText = hudObj.AddComponent<TextMeshProUGUI>();
        hudText.fontSize = 20;
        hudText.alignment = TextAlignmentOptions.Center;
        hudText.color = Color.white;
        UpdateHUD();
    }

    void Update()
    {
        HandleDayCycle();
        HandleInput();
    }

    void HandleDayCycle()
    {
        tagesTimer += Time.deltaTime;
        float progress = tagesTimer / tagesDauer;

        if (sunLight != null)
        {
            float angle = Mathf.Lerp(28f, 155f, progress);
            sunLight.transform.rotation = Quaternion.Euler(angle, -35f, 0f);
            sunLight.color = Color.Lerp(new Color(1f, 0.95f, 0.88f), new Color(1f, 0.58f, 0.35f), Mathf.Abs(progress - 0.5f) * 2f);
        }

        if (tagesTimer >= tagesDauer)
        {
            tagesTimer = 0f;
            EndDayAndSimulate();
        }

        UpdateHUD();
    }

    void EndDayAndSimulate()
    {
        tag++;
        marmor += 18;

        int boutiques = 0;
        int cafes = 0;
        int juweliere = 0;

        foreach (var t in builtShopTypes.Values)
        {
            if (t == GalleriaShopType.Boutique) boutiques++;
            if (t == GalleriaShopType.Cafe) cafes++;
            if (t == GalleriaShopType.Juwelier) juweliere++;
        }

        int kunden = flaneureAnzahl;

        // Einkäufe der Bürger
        int kaffee = Mathf.Min(kunden, cafes * 6);
        lire += kaffee * 9;

        int mode = Mathf.Min(kunden, boutiques * 4);
        seide += boutiques * 5;
        lire += mode * 18;
        luxuswaren += boutiques * 2;

        if (juweliere > 0)
            lire += juweliere * 40;

        flaneureAnzahl += Mathf.Clamp(boutiques + cafes * 2 + juweliere * 3, 2, 9);
        SpawnFlaneure();
    }

    void SpawnFlaneure()
    {
        foreach (var c in citizens) if (c != null) Destroy(c);
        citizens.Clear();

        for (int i = 0; i < Mathf.Min(flaneureAnzahl, 35); i++)
        {
            GameObject person = GameObject.CreatePrimitive(PrimitiveType.Capsule);
            person.name = "Flaneur";
            Destroy(person.GetComponent<Collider>());
            person.transform.localScale = new Vector3(0.35f, 0.5f, 0.35f);
            person.transform.position = new Vector3(Random.Range(-8f, 8f), 0.5f, Random.Range(-8f, 8f));

            Material pm = new Material(storefrontMat);
            pm.color = Color.HSVToRGB(Random.Range(0.04f, 0.14f), 0.55f, 0.85f);
            person.GetComponent<Renderer>().sharedMaterial = pm;

            citizens.Add(person);
            StartCoroutine(CitizenFlanieren(person.transform));
        }
    }

    IEnumerator CitizenFlanieren(Transform p)
    {
        while (p != null)
        {
            Vector3 target = new Vector3(Random.Range(-8f, 8f), 0.5f, Random.Range(-8f, 8f));
            if (builtShopObjects.Count > 0 && Random.value > 0.35f)
            {
                var keys = new List<Vector2Int>(builtShopObjects.Keys);
                Vector2Int cell = keys[Random.Range(0, keys.Count)];
                target = new Vector3(cell.x * 4.2f, 0.5f, cell.y * 4.2f);
            }

            while (p != null && Vector3.Distance(p.position, target) > 0.4f)
            {
                p.position = Vector3.MoveTowards(p.position, target, 2.3f * Time.deltaTime);
                yield return null;
            }
            yield return new WaitForSeconds(Random.Range(1.5f, 3.5f));
        }
    }

    void HandleInput()
    {
        if (Keyboard.current == null) return;

        if (Keyboard.current.iKey.wasPressedThisFrame) { cursor.y++; UpdateCursorPosition(); }
        if (Keyboard.current.kKey.wasPressedThisFrame) { cursor.y--; UpdateCursorPosition(); }
        if (Keyboard.current.jKey.wasPressedThisFrame) { cursor.x--; UpdateCursorPosition(); }
        if (Keyboard.current.lKey.wasPressedThisFrame) { cursor.x++; UpdateCursorPosition(); }

        if (Keyboard.current.digit1Key.wasPressedThisFrame) selectedShop = GalleriaShopType.Boutique;
        if (Keyboard.current.digit2Key.wasPressedThisFrame) selectedShop = GalleriaShopType.Cafe;
        if (Keyboard.current.digit3Key.wasPressedThisFrame) selectedShop = GalleriaShopType.Juwelier;

        if (Keyboard.current.spaceKey.wasPressedThisFrame)
        {
            TryBuildShop();
        }
    }

    void UpdateCursorPosition()
    {
        cursor.x = Mathf.Clamp(cursor.x, -2, 2);
        cursor.y = Mathf.Clamp(cursor.y, -2, 2);
        if (cursorMarker != null)
            cursorMarker.transform.position = new Vector3(cursor.x * 4.2f, 0.06f, cursor.y * 4.2f);
    }

    void TryBuildShop()
    {
        if (builtShopObjects.ContainsKey(cursor)) return;

        int kLire = selectedShop == GalleriaShopType.Boutique ? 70 : (selectedShop == GalleriaShopType.Cafe ? 45 : 130);
        int kMarmor = selectedShop == GalleriaShopType.Boutique ? 15 : (selectedShop == GalleriaShopType.Cafe ? 10 : 25);

        if (lire >= kLire && marmor >= kMarmor)
        {
            lire -= kLire;
            marmor -= kMarmor;
            TryBuildAt(cursor, selectedShop, false);
            UpdateHUD();
        }
    }

    void TryBuildAt(Vector2Int pos, GalleriaShopType type, bool free)
    {
        builtShopTypes[pos] = type;

        Vector3 worldPos = new Vector3(pos.x * 4.2f, 0f, pos.y * 4.2f);
        Material shopAccentMat = new Material(storefrontMat);
        shopAccentMat.color = type == GalleriaShopType.Boutique ? new Color(0.18f, 0.16f, 0.14f)
            : (type == GalleriaShopType.Cafe ? new Color(0.45f, 0.28f, 0.18f) : new Color(0.85f, 0.72f, 0.28f));

        GameObject shopObj = MilanoMeshGenerator.CreateArch($"Galleria_{type}_{pos.x}_{pos.y}", 3.4f, 3.6f, 1.2f, marbleMat, shopAccentMat);
        shopObj.transform.SetParent(transform, false);
        shopObj.transform.localPosition = worldPos;

        // Ausrichtung zur Passage
        if (Mathf.Abs(pos.x) > Mathf.Abs(pos.y))
            shopObj.transform.localRotation = Quaternion.Euler(0f, pos.x > 0 ? -90f : 90f, 0f);
        else
            shopObj.transform.localRotation = Quaternion.Euler(0f, pos.y > 0 ? 180f : 0f, 0f);

        builtShopObjects[pos] = shopObj;
    }

    void UpdateHUD()
    {
        if (hudText == null) return;
        string sel = selectedShop switch
        {
            GalleriaShopType.Boutique => "[1] Boutique (70L, 15M)",
            GalleriaShopType.Cafe => "[2] Caffè (45L, 10M)",
            GalleriaShopType.Juwelier => "[3] Juwelier (130L, 25M)",
            _ => ""
        };

        hudText.text = $"Galleria Vittorio Emanuele II | Tag {tag} | Lire: {lire} | Marmor: {marmor} | Seide: {seide} | Luxus: {luxuswaren} | Flaneure: {flaneureAnzahl} | Bau: {sel} [Leertaste]";
    }
}
