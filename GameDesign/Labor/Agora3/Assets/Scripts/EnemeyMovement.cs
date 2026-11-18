using UnityEngine;
using UnityEngine.AI;
using TMPro;

// Kunde (die roten Kugeln, früher die Gegner).
// Er kommt durch einen Eingang aufs Brett, läuft zu einem freien Platz an der
// Theke und zeigt in einer Sprechblase, was er haben will. Rollt der Ball mit
// dem passenden Produkt zu ihm, bezahlt er und geht zufrieden. Läuft seine
// Geduld ab, geht er verärgert (kostet Ruf).
//
// Bewegung wie bisher: Der NavMeshAgent rechnet auf dem festen NavMesh an der
// Ausgangslage des Bretts, die sichtbare Position wird um den Brett-Versatz verschoben.
public class EnemyMovement : MonoBehaviour
{
    public enum State { Arriving, Waiting, Leaving }

    // Reference to the player's transform.
    public Transform player;

    // Das bewegliche Spielfeld (wird automatisch gesucht, falls leer).
    public Transform board;

    [Tooltip("Abstand (waagerecht, Mitte zu Mitte), ab dem der Ball dem Kunden etwas übergeben kann")]
    public float catchDistance = 1.25f;

    [Header("Kunde")]
    public float walkSpeed = 3f;
    public float bubbleHeight = 2.1f;
    [Tooltip("Gesamtgröße der Sprechblase")]
    public float bubbleScale = 0.8f;
    public Color bubbleColorHappy = new Color(0.45f, 0.95f, 0.45f);
    public Color bubbleColorAngry = new Color(1f, 0.35f, 0.3f);

    public State CurrentState { get; private set; } = State.Arriving;
    public TradeCatalog.Product Wish { get; private set; }
    public float PatienceLeft => patience;
    public float Patience01 => maxPatience > 0f ? patience / maxPatience : 0f;
    public int CounterSlot { get; private set; } = -1;

    private BallGameLogic ball;
    private LevelManager shop;
    private NavMeshAgent navMeshAgent;
    private Vector3 boardStartPosition;
    private Vector3 counterNav, exitNav;
    private float patience, maxPatience;
    private int reward;

    private Transform bubble;
    private Material borderMaterial;
    private Material barMaterial;
    private Transform patienceBar;
    private float leaveTimer;

    // Awake statt Start: Der LevelManager klont Kunden und platziert sie
    // sofort danach mit Teleport() – dafür muss alles schon eingerichtet sein.
    void Awake()
    {
        navMeshAgent = GetComponent<NavMeshAgent>();
        navMeshAgent.autoBraking = true;
        navMeshAgent.stoppingDistance = 0.05f;
        navMeshAgent.acceleration = 20f;
        navMeshAgent.angularSpeed = 720f;

        if (player != null) ball = player.GetComponent<BallGameLogic>();

        PlaygroundMovement playground = board != null
            ? board.GetComponent<PlaygroundMovement>()
            : FindAnyObjectByType<PlaygroundMovement>();

        if (playground != null)
        {
            board = playground.transform;
            boardStartPosition = playground.StartPosition;
            navMeshAgent.updatePosition = false;
        }
        else board = null;
    }

    // Vom LevelManager direkt nach dem Klonen aufgerufen (NavMesh-Koordinaten)
    public void Setup(LevelManager shop, BallGameLogic ball, TradeCatalog.Product wish, float patience,
                      int reward, int counterSlot, Vector3 counterNav, Vector3 exitNav)
    {
        this.shop = shop;
        this.ball = ball;
        if (ball != null) player = ball.transform;
        Wish = wish;
        maxPatience = this.patience = patience;
        this.reward = reward;
        CounterSlot = counterSlot;
        this.counterNav = counterNav;
        this.exitNav = exitNav;

        navMeshAgent.speed = walkSpeed;
        CurrentState = State.Arriving;
        if (navMeshAgent.isOnNavMesh) navMeshAgent.SetDestination(counterNav);
        CreateBubble();
    }

    public void Teleport(Vector3 navMeshPosition)
    {
        navMeshAgent.Warp(navMeshPosition);
        if (board != null)
            transform.position = navMeshAgent.nextPosition + (board.position - boardStartPosition);
        // Nach einem NavMesh-Neubau das aktuelle Ziel wieder setzen
        if (navMeshAgent.isOnNavMesh && Wish != null)
            navMeshAgent.SetDestination(CurrentState == State.Leaving ? exitNav : counterNav);
    }

    void Update()
    {
        Vector3 boardDelta = board != null ? board.position - boardStartPosition : Vector3.zero;
        if (board != null) transform.position = navMeshAgent.nextPosition + boardDelta;

        if (Wish == null) return; // nur die Vorlage
        if (ball != null && ball.IsGameOver && CurrentState != State.Leaving) Leave(false);

        switch (CurrentState)
        {
            case State.Arriving:
                if (ReachedDestination()) CurrentState = State.Waiting;
                TryReceive(); // auch schon unterwegs annehmen
                break;

            case State.Waiting:
                patience -= Time.deltaTime;
                if (patience <= 0f) { Leave(true); break; }
                TryReceive();
                break;

            case State.Leaving:
                leaveTimer += Time.deltaTime;
                if (ReachedDestination() || leaveTimer > 12f)
                {
                    // Schrumpfen und verschwinden
                    transform.localScale = Vector3.MoveTowards(transform.localScale, Vector3.zero, 3f * Time.deltaTime);
                    if (transform.localScale.x <= 0.01f) Destroy(gameObject);
                }
                break;
        }

        UpdateBubble();
    }

    bool ReachedDestination()
    {
        if (!navMeshAgent.isOnNavMesh || navMeshAgent.pathPending) return false;
        return navMeshAgent.remainingDistance <= 0.15f;
    }

    // Ball nah genug und hat das gewünschte Produkt dabei?
    void TryReceive()
    {
        if (player == null || ball == null) return;
        Vector3 diff = player.position - transform.position;
        diff.y = 0f;
        if (diff.sqrMagnitude > catchDistance * catchDistance) return;

        int tip = Mathf.RoundToInt(Patience01 * 3f); // schnell bedient = Trinkgeld
        if (ball.TryDeliver(Wish.name, reward + tip))
        {
            if (shop != null) shop.OnCustomerServed(this, reward + tip);
            Leave(false);
        }
    }

    void Leave(bool angry)
    {
        if (CurrentState == State.Leaving) return;
        CurrentState = State.Leaving;
        if (angry && shop != null) shop.OnCustomerAngry(this);
        if (shop != null) shop.OnCustomerLeaving(this);
        if (navMeshAgent.isOnNavMesh) navMeshAgent.SetDestination(exitNav);
        if (bubble != null) Destroy(bubble.gameObject);
    }

    // ---------- Sprechblase ----------

    const float BubbleWidth = 3f, BubbleHeight = 1.8f;

    // Weiße Sprechblase mit farbigem Rand (= Geduld):
    //   oben groß der Name des Produkts
    //   darunter die nötigen Teile als farbige Kästchen mit Namen, dazwischen "+"
    //   unten der Geduld-Balken
    void CreateBubble()
    {
        bubble = new GameObject("WishBubble").transform;
        bubble.SetParent(transform, false);
        bubble.localPosition = Vector3.up * bubbleHeight;
        bubble.localScale = Vector3.one * bubbleScale;

        borderMaterial = UnlitMaterial(bubbleColorHappy);
        Material white = UnlitMaterial(Color.white);

        // Rand (etwas größer, liegt hinter dem weißen Feld)
        Quad("Border", new Vector3(0f, 0f, 0.02f), new Vector2(BubbleWidth + 0.16f, BubbleHeight + 0.16f), borderMaterial);
        Quad("Background", Vector3.zero, new Vector2(BubbleWidth, BubbleHeight), white);
        // Zipfel nach unten zum Kunden (wie in der Skizze)
        Quad("Tail", new Vector3(0f, -BubbleHeight / 2f - 0.05f, 0.02f), new Vector2(0.45f, 0.45f), borderMaterial)
            .localRotation = Quaternion.Euler(0f, 0f, 45f);

        // Produktname
        Label(Wish.name, new Vector3(0f, 0.55f, -0.02f), 4.2f, true);

        // Teile als Kästchen
        int n = Wish.parts.Length;
        float spacing = 0.95f;
        for (int i = 0; i < n; i++)
        {
            float x = (i - (n - 1) / 2f) * spacing;
            string part = Wish.parts[i];
            Quad("Icon " + part, new Vector3(x, 0f, -0.01f), new Vector2(0.42f, 0.42f),
                 UnlitMaterial(TradeCatalog.ColorOf(part)));
            Label(part, new Vector3(x, -0.38f, -0.02f), 2.2f, false);
            if (i < n - 1) Label("+", new Vector3(x + spacing / 2f, 0f, -0.02f), 3.5f, true);
        }

        // Geduld-Balken
        barMaterial = UnlitMaterial(bubbleColorHappy);
        patienceBar = Quad("PatienceBar", new Vector3(0f, -0.72f, -0.01f), new Vector2(BubbleWidth - 0.3f, 0.12f), barMaterial);
    }

    Transform Quad(string name, Vector3 localPos, Vector2 size, Material mat)
    {
        GameObject q = GameObject.CreatePrimitive(PrimitiveType.Quad);
        q.name = name;
        Destroy(q.GetComponent<Collider>());
        q.transform.SetParent(bubble, false);
        q.transform.localPosition = localPos;
        q.transform.localScale = new Vector3(size.x, size.y, 1f);
        Renderer r = q.GetComponent<Renderer>();
        r.sharedMaterial = mat;
        r.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;
        return q.transform;
    }

    void Label(string text, Vector3 localPos, float size, bool bold)
    {
        GameObject go = new GameObject("Text");
        go.transform.SetParent(bubble, false);
        go.transform.localPosition = localPos;
        TextMeshPro tmp = go.AddComponent<TextMeshPro>();
        tmp.rectTransform.sizeDelta = new Vector2(BubbleWidth, 0.6f);
        tmp.alignment = TextAlignmentOptions.Center;
        tmp.textWrappingMode = TextWrappingModes.NoWrap;
        tmp.fontSize = size;
        tmp.color = new Color(0.1f, 0.1f, 0.1f);
        tmp.text = bold ? "<b>" + text + "</b>" : text;
    }

    // Unbeleuchtet, damit die Farben in der Blase klar und hell bleiben
    static Material UnlitMaterial(Color color)
    {
        Shader shader = Shader.Find("Universal Render Pipeline/Unlit");
        if (shader == null) shader = Shader.Find("Unlit/Color");
        Material m = shader != null ? new Material(shader) : new Material(Shader.Find("Standard"));
        m.color = color;
        if (m.HasProperty("_BaseColor")) m.SetColor("_BaseColor", color);
        return m;
    }

    void SetColor(Material m, Color c)
    {
        m.color = c;
        if (m.HasProperty("_BaseColor")) m.SetColor("_BaseColor", c);
    }

    void UpdateBubble()
    {
        if (bubble == null) return;
        Camera cam = Camera.main;
        if (cam != null) bubble.rotation = cam.transform.rotation;

        float k = Patience01;
        Color c = Color.Lerp(bubbleColorAngry, bubbleColorHappy, k);
        SetColor(borderMaterial, c);
        SetColor(barMaterial, c);
        float full = BubbleWidth - 0.3f;
        patienceBar.localScale = new Vector3(Mathf.Max(0.001f, full * k), 0.12f, 1f);
        patienceBar.localPosition = new Vector3(-full / 2f * (1f - k), -0.72f, -0.01f);

        // Kurz vor Schluss wippt die Blase ungeduldig
        float wobble = k < 0.25f ? Mathf.Sin(Time.time * 20f) * 0.06f : 0f;
        bubble.localPosition = Vector3.up * (bubbleHeight + wobble);
    }
}
