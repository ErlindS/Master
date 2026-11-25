using System.Collections.Generic;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.SceneManagement;
using TMPro;

// Der Ball ist der Händler: Er trägt ein kleines Inventar (Teile und fertige
// Produkte), hat Münzen und einen Ruf. Unzufriedene Kunden kosten Ruf.
//   Gewonnen: coinsToWin Münzen verdient
//   Verloren: Ruf ist auf 0
// Die getragenen Dinge kreisen als kleine Würfel (Teile) bzw. Kugeln (Produkte) um den Ball.
public class BallGameLogic : MonoBehaviour
{
    public TextMeshProUGUI countText;
    public GameObject winTextObject;

    [Header("Handel")]
    public int startCoins = 6;
    public int coinsToWin = 60;
    [Tooltip("So viele Dinge (Teile + Produkte) kann der Ball gleichzeitig tragen")]
    public int maxInventory = 4;

    [Header("Ruf (Leben)")]
    [Tooltip("So viele Kunden dürfen unzufrieden gehen, bevor das Spiel verloren ist")]
    public int lives = 3;

    [Header("Inventar-Anzeige")]
    public float orbitRadius = 0.85f;
    public float orbitHeight = 0.55f;
    public float orbitSpeed = 90f;

    // Ein Kunde ist unzufrieden gegangen (Kamera wackelt)
    public event System.Action Hit;

    public int Coins { get; private set; }
    public int Served { get; private set; }
    public IReadOnlyList<string> Inventory => inventory;
    public bool IsFull => inventory.Count >= maxInventory;
    public bool HasWon { get; private set; }
    public bool HasLost { get; private set; }
    public bool IsGameOver => HasWon || HasLost;

    private readonly List<string> inventory = new List<string>();
    private readonly List<GameObject> inventoryVisuals = new List<GameObject>();
    private Transform orbit;
    private Material baseMaterial;

    void Awake()
    {
        Coins = startCoins;
        orbit = new GameObject("InventoryOrbit").transform;

        GameObject temp = GameObject.CreatePrimitive(PrimitiveType.Cube);
        baseMaterial = temp.GetComponent<Renderer>().sharedMaterial;
        Destroy(temp);
    }

    void Start()
    {
        FindMissingUi();
        if (winTextObject != null) winTextObject.SetActive(false);
        if (countText != null)
        {
            // Platz für zwei lange Zeilen (in der Szene ist das Feld nur 200 px breit)
            countText.rectTransform.sizeDelta = new Vector2(900f, 90f);
            countText.textWrappingMode = TextWrappingModes.NoWrap;
            countText.outlineWidth = 0.15f;
            countText.outlineColor = Color.black;
        }
        RefreshUi();
    }

    void OnDestroy()
    {
        if (orbit != null) Destroy(orbit.gameObject);
    }

    // Falls die Verknüpfungen im Inspector verloren gegangen sind:
    // Texte über ihren Namen in der Szene suchen ("CountText" / "WinText").
    void FindMissingUi()
    {
        if (countText != null && winTextObject != null) return;

        TextMeshProUGUI[] texts = FindObjectsByType<TextMeshProUGUI>(FindObjectsInactive.Include);
        foreach (TextMeshProUGUI t in texts)
        {
            if (countText == null && t.name == "CountText") countText = t;
            if (winTextObject == null && t.name == "WinText") winTextObject = t.gameObject;
        }

        if (countText == null || winTextObject == null)
            Debug.LogWarning("BallGameLogic: CountText oder WinText nicht gefunden – bitte im Inspector zuweisen.", this);
    }

    void Update()
    {
        Keyboard kb = Keyboard.current;
        Gamepad pad = Gamepad.current;

        // Q / Gamepad-West: ältestes Ding wegwerfen (falls man sich verkauft hat)
        bool drop = (kb != null && (kb.qKey.wasPressedThisFrame || kb.backspaceKey.wasPressedThisFrame))
                    || (pad != null && pad.buttonWest.wasPressedThisFrame);
        if (drop && !IsGameOver && inventory.Count > 0)
        {
            inventory.RemoveAt(0);
            InventoryChanged();
        }

        // R: nach Spielende neu starten
        bool restart = (kb != null && kb.rKey.wasPressedThisFrame) || (pad != null && pad.startButton.wasPressedThisFrame);
        if (restart && IsGameOver)
            SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);

        UpdateOrbit();
    }

    // ---------- Inventar ----------

    public bool TryAdd(string item)
    {
        if (IsFull || IsGameOver) return false;
        inventory.Add(item);
        InventoryChanged();
        return true;
    }

    public int CountOf(string item)
    {
        int n = 0;
        foreach (string i in inventory) if (i == item) n++;
        return n;
    }

    public bool Has(string item) => inventory.Contains(item);

    // Sind alle Teile des Rezepts (mit Mehrfachnennung) im Inventar?
    public bool HasAll(IList<string> parts)
    {
        List<string> rest = new List<string>(inventory);
        foreach (string p in parts)
            if (!rest.Remove(p)) return false;
        return true;
    }

    // Teile, die für das Rezept noch fehlen
    public List<string> Missing(IList<string> parts)
    {
        List<string> rest = new List<string>(inventory);
        List<string> missing = new List<string>();
        foreach (string p in parts)
            if (!rest.Remove(p)) missing.Add(p);
        return missing;
    }

    // Ersetzt die Teile durch das fertige Produkt
    public bool Assemble(TradeCatalog.Product product)
    {
        if (!HasAll(product.parts)) return false;
        foreach (string p in product.parts) inventory.Remove(p);
        inventory.Add(product.name);
        InventoryChanged();
        return true;
    }

    // ---------- Geld & Kunden ----------

    public bool TryPay(int amount)
    {
        if (Coins < amount || IsGameOver) return false;
        Coins -= amount;
        RefreshUi();
        return true;
    }

    // Kunde bekommt sein Produkt -> Münzen
    public bool TryDeliver(string product, int reward)
    {
        if (IsGameOver || !inventory.Remove(product)) return false;
        Coins += reward;
        Served++;
        InventoryChanged();
        if (Coins >= coinsToWin) EndGame(true);
        return true;
    }

    // Kunde ist unzufrieden gegangen
    public void LoseReputation()
    {
        if (IsGameOver) return;
        lives = Mathf.Max(0, lives - 1);
        Hit?.Invoke();
        RefreshUi();
        if (lives <= 0) EndGame(false);
    }

    void EndGame(bool won)
    {
        HasWon = won;
        HasLost = !won;
        RefreshUi();
        if (winTextObject != null)
        {
            winTextObject.SetActive(true);
            winTextObject.GetComponent<TextMeshProUGUI>().text = won
                ? "Gewonnen!\n<size=50%>" + Served + " Kunden bedient – R für neues Spiel</size>"
                : "Verloren!\n<size=50%>Zu viele unzufriedene Kunden – R für neues Spiel</size>";
        }
    }

    // ---------- Anzeige ----------

    void InventoryChanged()
    {
        RebuildVisuals();
        RefreshUi();
    }

    void RefreshUi()
    {
        if (countText == null) return;

        // Anzeige oben links (Münzen / Ruf / Bedient / Inventar) entfernt (auf Wunsch)
        countText.text = "";
    }

    void RebuildVisuals()
    {
        foreach (GameObject v in inventoryVisuals) Destroy(v);
        inventoryVisuals.Clear();

        foreach (string item in inventory)
        {
            bool part = TradeCatalog.IsPart(item);
            GameObject v = GameObject.CreatePrimitive(part ? PrimitiveType.Cube : PrimitiveType.Sphere);
            v.name = "Carry " + item;
            Destroy(v.GetComponent<Collider>());
            v.transform.SetParent(orbit, false);
            v.transform.localScale = Vector3.one * (part ? 0.28f : 0.4f);
            Material m = new Material(baseMaterial) { color = TradeCatalog.ColorOf(item) };
            if (!part && m.HasProperty("_EmissionColor"))
            {
                m.EnableKeyword("_EMISSION");
                m.SetColor("_EmissionColor", TradeCatalog.ColorOf(item) * 0.4f);
            }
            v.GetComponent<Renderer>().material = m;
            inventoryVisuals.Add(v);
        }
    }

    void UpdateOrbit()
    {
        orbit.position = transform.position + Vector3.up * orbitHeight;
        int n = inventoryVisuals.Count;
        for (int i = 0; i < n; i++)
        {
            float angle = Time.time * orbitSpeed + i * 360f / n;
            Vector3 offset = Quaternion.Euler(0f, angle, 0f) * Vector3.forward * orbitRadius;
            inventoryVisuals[i].transform.localPosition = offset;
            inventoryVisuals[i].transform.localRotation = Quaternion.Euler(0f, angle * 2f, 0f);
        }
    }
}
