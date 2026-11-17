using System.Collections;
using System.Collections.Generic;
using Unity.AI.Navigation;
using UnityEngine;
using UnityEngine.AI;

// Stufen-System: Es liegt immer nur EIN Pickup auf dem Spielfeld.
// Sobald er eingesammelt ist, wird die nächste Stufe aufgebaut – das Spielfeld
// wird dabei NICHT zurückgesetzt, sondern nur schwerer.
// Wände kommen aus den Karten, die Gegner aus einer EXPONENTIELLEN Kurve:
//   Anzahl  = enemyCountGrowth ^ (Stufe - firstEnemyStage)   (gerundet)
//   Tempo   = enemyBaseSpeed * enemySpeedGrowth ^ (Stufe - firstEnemyStage)
// Die ersten Stufen sind also ganz ohne Gegner, danach zieht es immer stärker an.
// Gehört auf das Spielfeld-Objekt ("Walls"), zusammen mit PlaygroundMovement.
//
// Legende der Karten (20 x 20 Zeichen, 1 Zeichen = 1 Unity-Einheit).
// Jede Karte enthält nur das, was in DIESER Stufe NEU dazukommt:
//   #  neue Mauer
//   .  frei
//   o  der Pickup dieser Stufe (genau einer)
//   S  Startposition des Balls (nur in der ersten Stufe)
//   E  (optional) bevorzugter Spawnpunkt für neue Gegner
[RequireComponent(typeof(PlaygroundMovement))]
public class LevelManager : MonoBehaviour
{
    [System.Serializable]
    public class Stage
    {
        public string name;
        [Tooltip("Korrektur der Gegnerzahl gegenüber der Kurve (z. B. -1 = ein Gegner weniger)")]
        public int enemyCountOffset = 0;
        [Tooltip("Faktor auf das Gegner-Tempo der Kurve (z. B. 0.85 = 15 % langsamer)")]
        public float enemySpeedFactor = 1f;
        [TextArea(20, 22)]
        public string map;
    }

    public List<Stage> stages = new List<Stage>
    {
        new Stage
        {
            name = "Stufe 1: Aufwärmen",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "..............o.....\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                ".........S..........\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 2: Eine Wand",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...#########........\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....o...............\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 3: Mehr Wände",
            map =
                "....................\n" +
                "....................\n" +
                "...............#.o..\n" +
                "...............#....\n" +
                "...............#....\n" +
                "...............#....\n" +
                "...............#....\n" +
                "...............#....\n" +
                "...............#....\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "........#########...\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 4: Der erste Gegner",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                ".................o..\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 5: Engpass",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....#...............\n" +
                "....#...............\n" +
                ".o..#...............\n" +
                "....#...............\n" +
                "....#...............\n" +
                "....#...............\n" +
                "....#...............\n" +
                "....#...............\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 6: Zu zweit",
            map =
                "....................\n" +
                "....................\n" +
                "..o.................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "##..................\n" +
                "....................\n" +
                "....................\n" +
                "...........#######..\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 7: Gedränge",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                ".................o..\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 8: Sackgassen",
            enemyCountOffset = -1,
            enemySpeedFactor = 0.85f,
            map =
                ".........#..........\n" +
                ".........#..........\n" +
                ".........#..........\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "............#.......\n" +
                "............#.o.....\n" +
                "............#......."
        },
        new Stage
        {
            name = "Stufe 9: Labyrinth",
            enemyCountOffset = -1,
            enemySpeedFactor = 0.85f,
            map =
                "....................\n" +
                "...........o........\n" +
                "....###.............\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                ".......##...........\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 10: Jagd",
            enemyCountOffset = -2,
            enemySpeedFactor = 0.85f,
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "..................##\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "......o.............\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 11: Überzahl",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "........o...........\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "...............##...\n" +
                "....................\n" +
                "...................."
        },
        new Stage
        {
            name = "Stufe 12: Finale",
            map =
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                "....................\n" +
                ".o..................\n" +
                "...................."
        }
    };

    [Header("Schwierigkeit (exponentiell)")]
    [Tooltip("In dieser Stufe (1 = erste) taucht der erste Gegner auf")]
    public int firstEnemyStage = 4;
    [Tooltip("Faktor, um den die Gegnerzahl pro Stufe wächst")]
    public float enemyCountGrowth = 1.3f;
    public int maxEnemies = 4;
    [Tooltip("Tempo der Gegner in der ersten Gegner-Stufe")]
    public float enemyBaseSpeed = 2.5f;
    [Tooltip("Faktor, um den das Gegner-Tempo pro Stufe wächst")]
    public float enemySpeedGrowth = 1.12f;
    public float maxEnemySpeed = 7f;

    [Header("Ablauf")]
    [Tooltip("Kurze Pause nach dem Einsammeln, bevor die nächste Stufe erscheint")]
    public float pauseBetweenStages = 0.6f;
    [Tooltip("Gegner spawnen mindestens so weit vom Ball entfernt")]
    public float minEnemySpawnDistance = 7f;

    [Header("Spawn-Warnung")]
    [Tooltip("So lange blinkt eine rote Markierung, bevor dort ein Gegner erscheint")]
    public float spawnWarningTime = 1.5f;
    public Color spawnWarningColor = new Color(1f, 0.15f, 0.1f);

    [Header("Treffer")]
    [Tooltip("Nach einem Treffer werden alle Gegner mindestens so weit vom Ball weggesetzt")]
    public float pushBackDistance = 8f;


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

    [Header("Haltestange (hält den Ball fest)")]
    public bool createHolderRod = true;
    [Tooltip("Höhe des waagerechten Arms über dem Brett (muss über den Mauern liegen)")]
    public float rodHeight = 2.5f;
    public float rodThickness = 0.25f;
    public Color rodColor = new Color(0.75f, 0.77f, 0.8f);

    [Header("Referenzen (werden automatisch gesucht, wenn leer)")]
    public Transform ground;
    public Transform pickUpParent;
    public GameObject pickUpTemplate;
    public Material wallMaterial;
    public BallGameLogic ball;
    [Tooltip("Vorlage für alle Gegner (wird geklont)")]
    public EnemyMovement enemy;

    private const int GridSize = 20;
    private const float CellSize = 1f;

    private PlaygroundMovement playground;
    private NavMeshSurface navMeshSurface;
    // Unser eigenes, FEST stehendes NavMesh (siehe RebuildNavMesh)
    private NavMeshDataInstance fixedNavMesh;

    private readonly List<GameObject> spawnedWalls = new List<GameObject>();
    private readonly List<EnemyMovement> spawnedEnemies = new List<EnemyMovement>();
    private readonly bool[,] wallGrid = new bool[GridSize, GridSize];
    private GameObject currentPickUp;

    // Aktueller (noch nicht eingesammelter) Pickup – z. B. für die Kamera
    public Transform CurrentPickUp =>
        currentPickUp != null && currentPickUp.activeSelf ? currentPickUp.transform : null;
    private float currentEnemySpeed = 2.5f;
    private int pendingEnemies; // Gegner, deren Spawn-Warnung gerade läuft
    private int currentStage = -1;
    private bool switching;

    void Start()
    {
        playground = GetComponent<PlaygroundMovement>();
        FindReferences();

        // Alte, von Hand platzierte Hindernisse und Pickups ausblenden
        if (ground != null)
            foreach (Transform child in ground)
                child.gameObject.SetActive(false);
        if (pickUpParent != null)
            foreach (Transform child in pickUpParent)
                child.gameObject.SetActive(false);

        // WICHTIG: Die NavMeshSurface hängt am beweglichen Brett. Aktiv würde sie
        // ihr NavMesh in jedem Frame entfernen und an der neuen Brett-Position
        // neu einsetzen -> Gegner verlieren ständig ihren Weg und laufen doppelt
        // versetzt. Wir schalten sie ab und setzen das NavMesh selbst fest ein.
        if (navMeshSurface != null) navMeshSurface.enabled = false;

        // Der Gegner in der Szene dient nur als Vorlage für die Klone
        if (enemy != null) enemy.gameObject.SetActive(false);

        if (createReferenceTable) CreateReferenceTable();
        if (createHolderRod) CreateHolderRod();

        if (ball != null)
        {
            ball.SetPickUpsToWin(stages.Count);
            ball.Hit += PushEnemiesAway;
        }

        LoadStage(0);
    }

    // Feste Stange, an der der Ball hängt: senkrechter Stift vom Ball nach oben,
    // waagerechter Arm über das Brett hinweg zu einem Pfosten neben dem Spielfeld.
    // Nicht Kind des Spielfelds -> bleibt stehen.
    void CreateHolderRod()
    {
        Vector3 pin = playground.PinPosition;
        float tableY = playground.StartPosition.y - tableDepth;
        float armY = playground.StartPosition.y + rodHeight;

        // Pfosten so weit weg, dass das Brett auch ganz verschoben nicht dagegen fährt
        float boardHalf = GridSize * CellSize / 2f + 0.25f;
        float postZ = pin.z - (2f * boardHalf + 1.5f);

        GameObject temp = GameObject.CreatePrimitive(PrimitiveType.Cube);
        Material rodMat = new Material(temp.GetComponent<Renderer>().sharedMaterial);
        Destroy(temp);
        rodMat.color = rodColor;
        if (rodMat.HasProperty("_Metallic")) rodMat.SetFloat("_Metallic", 0.8f);
        if (rodMat.HasProperty("_Smoothness")) rodMat.SetFloat("_Smoothness", 0.7f);

        GameObject holder = new GameObject("HolderRod");

        // Senkrechter Stift: von der Ballmitte bis zum Arm
        Vector3 pinBottom = pin;
        Vector3 pinTop = new Vector3(pin.x, armY, pin.z);
        CreateRodSegment(holder.transform, pinBottom, pinTop, rodThickness * 0.6f, rodMat);

        // Waagerechter Arm
        Vector3 armEnd = new Vector3(pin.x, armY, postZ);
        CreateRodSegment(holder.transform, pinTop, armEnd, rodThickness, rodMat);

        // Pfosten bis auf den Tisch
        Vector3 postBottom = new Vector3(pin.x, tableY, postZ);
        CreateRodSegment(holder.transform, armEnd, postBottom, rodThickness * 2f, rodMat);

        // Gelenk-Kugeln an den Knickstellen (sieht sauberer aus)
        CreateJoint(holder.transform, pinTop, rodThickness * 1.6f, rodMat);
        CreateJoint(holder.transform, armEnd, rodThickness * 2.4f, rodMat);
    }

    void CreateRodSegment(Transform parent, Vector3 from, Vector3 to, float thickness, Material mat)
    {
        GameObject rod = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        rod.name = "Rod";
        Destroy(rod.GetComponent<Collider>()); // reine Optik
        rod.transform.SetParent(parent, false);
        rod.transform.position = (from + to) / 2f;
        rod.transform.up = (to - from).normalized;
        // Cylinder ist 2 Einheiten hoch
        rod.transform.localScale = new Vector3(thickness, Vector3.Distance(from, to) / 2f, thickness);
        rod.GetComponent<Renderer>().sharedMaterial = mat;
    }

    void CreateJoint(Transform parent, Vector3 position, float size, Material mat)
    {
        GameObject joint = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        joint.name = "RodJoint";
        Destroy(joint.GetComponent<Collider>());
        joint.transform.SetParent(parent, false);
        joint.transform.position = position;
        joint.transform.localScale = Vector3.one * size;
        joint.GetComponent<Renderer>().sharedMaterial = mat;
    }

    // Steckt ein Kreis (Ball) an dieser Stelle (relativ zum Brett) in einer Mauer?
    public bool IsBlocked(Vector3 boardLocal, float radius)
    {
        float half = GridSize * CellSize / 2f;
        float inner = half - 0.25f; // Innenkante der Außenmauern

        if (Mathf.Abs(boardLocal.x) > inner - radius || Mathf.Abs(boardLocal.z) > inner - radius)
            return true;

        // Nur Zellen in der Nähe des Balls prüfen
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

    // Fester Tisch mit Schachbrettmuster unter dem Spielfeld.
    // Er ist KEIN Kind des Spielfelds und bleibt deshalb stehen. Gegen das
    // Muster sieht man sofort, dass sich das Brett bewegt und nicht der Ball.
    void CreateReferenceTable()
    {
        Vector3 center = playground.StartPosition;
        float tableY = center.y - tableDepth;

        GameObject table = new GameObject("ReferenceTable");
        table.transform.position = new Vector3(center.x, tableY, center.z);

        // Schachbrett-Textur (2x2 Pixel, wird gekachelt)
        Texture2D checker = new Texture2D(2, 2) { filterMode = FilterMode.Point, wrapMode = TextureWrapMode.Repeat };
        checker.SetPixels(new[] { tableColorA, tableColorB, tableColorB, tableColorA });
        checker.Apply();

        GameObject top = GameObject.CreatePrimitive(PrimitiveType.Plane);
        top.name = "TableTop";
        Destroy(top.GetComponent<Collider>()); // reine Optik
        top.transform.SetParent(table.transform, false);
        top.transform.localScale = new Vector3(tableSize / 10f, 1f, tableSize / 10f); // Plane ist 10x10
        Renderer topRenderer = top.GetComponent<Renderer>();
        Material topMat = new Material(topRenderer.sharedMaterial);
        topMat.mainTexture = checker;
        topMat.color = Color.white;
        // Ein Pixel der Textur = ein Feld -> 2 Felder pro Kachel
        float tiles = tableSize / (tileSize * 2f);
        topMat.mainTextureScale = new Vector2(tiles, tiles);
        topRenderer.sharedMaterial = topMat;

        // Rahmen, der die Ruheposition des Bretts markiert
        Material frameMat = new Material(topRenderer.sharedMaterial) { mainTexture = null, color = homeFrameColor };
        float half = GridSize * CellSize / 2f + 0.5f; // etwas größer als das Brett inkl. Außenmauern
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

    void FindReferences()
    {
        if (ground == null) ground = transform.Find("Ground");
        if (pickUpParent == null) pickUpParent = transform.Find("PickUpParent");
        if (pickUpTemplate == null && pickUpParent != null && pickUpParent.childCount > 0)
            pickUpTemplate = pickUpParent.GetChild(0).gameObject;
        if (wallMaterial == null)
        {
            Transform northWall = transform.Find("North Wall");
            if (northWall != null) wallMaterial = northWall.GetComponent<Renderer>().sharedMaterial;
        }
        if (ball == null) ball = FindAnyObjectByType<BallGameLogic>();
        if (enemy == null) enemy = FindAnyObjectByType<EnemyMovement>();

        if (ground != null) navMeshSurface = ground.GetComponent<NavMeshSurface>();
    }

    void Update()
    {
        if (switching || ball == null || currentPickUp == null) return;
        if (currentPickUp.activeSelf) return; // noch nicht eingesammelt

        if (currentStage + 1 < stages.Count)
            StartCoroutine(NextStageAfterPause());
        else
        {
            // Letzte Stufe geschafft -> BallGameLogic zeigt "You Win!".
            // Gegner verschwinden, damit man nach dem Sieg nicht noch verliert.
            currentPickUp = null;
            foreach (EnemyMovement e in spawnedEnemies)
                if (e != null) e.gameObject.SetActive(false);
        }
    }

    IEnumerator NextStageAfterPause()
    {
        switching = true;
        yield return new WaitForSeconds(pauseBetweenStages);
        if (ball != null) LoadStage(currentStage + 1);
        switching = false;
    }

    // Baut die Stufe auf: neue Mauern, neue Gegner und der nächste Pickup.
    // Alles Bisherige bleibt stehen.
    void LoadStage(int index)
    {
        currentStage = index;
        Stage stage = stages[index];

        if (currentPickUp != null) Destroy(currentPickUp);
        currentPickUp = null;

        string[] rows = stage.map.Replace("\r", "").Trim('\n').Split('\n');
        Vector2? pickUpCell = null;
        List<Vector2> enemyCells = new List<Vector2>();
        List<GameObject> newWalls = new List<GameObject>();

        for (int r = 0; r < rows.Length && r < GridSize; r++)
        {
            string row = rows[r];
            int c = 0;
            while (c < row.Length && c < GridSize)
            {
                if (row[c] == '#')
                {
                    // Nebeneinanderliegende Mauerstücke zu einem Block zusammenfassen.
                    // Zellen, in denen gerade der Ball steht, werden ausgelassen.
                    int start = -1;
                    while (c < row.Length && c < GridSize && row[c] == '#')
                    {
                        if (!BallOverlapsCell(r, c))
                        {
                            if (start < 0) start = c;
                            wallGrid[r, c] = true;
                        }
                        else if (start >= 0)
                        {
                            newWalls.Add(SpawnWall(r, start, c - start));
                            start = -1;
                        }
                        c++;
                    }
                    if (start >= 0) newWalls.Add(SpawnWall(r, start, c - start));
                    continue;
                }
                if (row[c] == 'o') pickUpCell = new Vector2(r, c);
                if (row[c] == 'E') enemyCells.Add(new Vector2(r, c));
                if (row[c] == 'S' && index == 0)
                    playground.CenterBoardOn(CellToBoardLocal(r, c, 0.5f)); // Startpunkt unter den Ball schieben
                c++;
            }
        }

        // NavMesh neu backen, damit die Gegner die neuen Mauern kennen
        // (vor der Wachs-Animation, solange die Mauern volle Höhe haben)
        RebuildNavMesh();
        foreach (GameObject wall in newWalls) StartCoroutine(RiseWall(wall.transform));

        // Gegner nach der exponentiellen Kurve
        currentEnemySpeed = EnemySpeedForStage(index);
        foreach (EnemyMovement e in spawnedEnemies)
            e.GetComponent<NavMeshAgent>().speed = currentEnemySpeed;

        int missing = EnemyCountForStage(index) - (spawnedEnemies.Count + pendingEnemies);
        for (int n = 0; n < missing; n++)
        {
            if (n < enemyCells.Count) SpawnEnemy((int)enemyCells[n].x, (int)enemyCells[n].y);
            else SpawnEnemyAnywhere();
        }

        Vector3 pickUpLocal = Vector3.zero;
        if (pickUpCell.HasValue)
        {
            pickUpLocal = CellToBoardLocal((int)pickUpCell.Value.x, (int)pickUpCell.Value.y, 0.5f);
            SpawnPickUp(pickUpLocal);
        }
        else
            Debug.LogWarning($"LevelManager: Stufe '{stage.name}' hat keinen Pickup ('o')!");

    }

    // Stufe als Index (0 = erste Stufe)
    public int EnemyCountForStage(int index)
    {
        int k = index + 1 - firstEnemyStage;
        if (k < 0) return 0;
        int count = Mathf.RoundToInt(Mathf.Pow(enemyCountGrowth, k)) + stages[index].enemyCountOffset;
        // Mindestens so viele wie in der Stufe davor (Gegner verschwinden nicht wieder)
        int previous = index > 0 ? EnemyCountForStage(index - 1) : 0;
        return Mathf.Clamp(Mathf.Max(count, previous), 0, maxEnemies);
    }

    public float EnemySpeedForStage(int index)
    {
        int k = Mathf.Max(0, index + 1 - firstEnemyStage);
        float speed = enemyBaseSpeed * Mathf.Pow(enemySpeedGrowth, k) * stages[index].enemySpeedFactor;
        return Mathf.Min(maxEnemySpeed, speed);
    }

    void OnDestroy()
    {
        fixedNavMesh.Remove();
        if (ball != null)
        {
            ball.Hit -= PushEnemiesAway;
        }
    }

    // Nach einem Treffer: alle Gegner weit weg vom Ball setzen (faire zweite Chance)
    void PushEnemiesAway()
    {
        Vector3 ballLocal = playground.PinPosition - transform.position;
        foreach (EnemyMovement e in spawnedEnemies)
        {
            if (e == null) continue;
            Vector3 navPos = playground.StartPosition + RandomFreeCellAwayFrom(ballLocal, pushBackDistance);
            if (NavMesh.SamplePosition(navPos, out NavMeshHit hit, 2f, NavMesh.AllAreas))
                e.Teleport(hit.position);
        }
    }

    // Zufällige freie Zelle (mit Platz drumherum) mit Mindestabstand zum Punkt
    Vector3 RandomFreeCellAwayFrom(Vector3 boardLocal, float minDistance)
    {
        for (int tries = 0; tries < 200; tries++)
        {
            int r = Random.Range(1, GridSize - 1);
            int c = Random.Range(1, GridSize - 1);
            if (wallGrid[r, c] || wallGrid[r - 1, c] || wallGrid[r + 1, c] || wallGrid[r, c - 1] || wallGrid[r, c + 1])
                continue;
            Vector3 p = CellToBoardLocal(r, c, 0f);
            if (Vector2.Distance(new Vector2(p.x, p.z), new Vector2(boardLocal.x, boardLocal.z)) >= minDistance)
                return p;
        }
        return FarthestFreeCellFrom(boardLocal);
    }

    // Steht der Ball (ganz oder teilweise) in dieser Zelle?
    bool BallOverlapsCell(int row, int col)
    {
        Vector3 ballLocal = playground.PinPosition - transform.position;
        Vector3 cell = CellToBoardLocal(row, col, 0f);
        float reach = CellSize / 2f + playground.ballCollisionRadius + 0.1f;
        return Mathf.Abs(ballLocal.x - cell.x) < reach && Mathf.Abs(ballLocal.z - cell.z) < reach;
    }

    void RebuildNavMesh()
    {
        if (navMeshSurface == null) return;

        // Das NavMesh liegt immer an der Ausgangsposition des Bretts (siehe EnemyMovement)
        // und bewegt sich NICHT mit – die Gegner rechnen den Versatz selbst ein.
        playground.RunWithBoardAtStart(() =>
        {
            navMeshSurface.BuildNavMesh();
            fixedNavMesh.Remove();
            fixedNavMesh = NavMesh.AddNavMeshData(navMeshSurface.navMeshData,
                navMeshSurface.transform.position, navMeshSurface.transform.rotation);
        });

        // Vorhandene Gegner wieder sauber aufs (neue) NavMesh setzen
        foreach (EnemyMovement e in spawnedEnemies)
        {
            NavMeshAgent agent = e.GetComponent<NavMeshAgent>();
            if (NavMesh.SamplePosition(agent.nextPosition, out NavMeshHit hit, 3f, NavMesh.AllAreas))
                e.Teleport(hit.position);
        }
    }

    // Brett-relative Position (Brett in Ausgangslage, ohne Drehung) einer Zelle
    Vector3 CellToBoardLocal(int row, float col, float height)
    {
        float half = GridSize * CellSize / 2f;
        return new Vector3(-half + (col + 0.5f) * CellSize, height, half - (row + 0.5f) * CellSize);
    }

    // Weltposition einer Zelle bei der AKTUELLEN Brett-Position
    Vector3 CellToWorld(int row, float col, float height)
    {
        return transform.position + CellToBoardLocal(row, col, height);
    }

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
        spawnedWalls.Add(wall);
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

    void SpawnPickUp(Vector3 boardLocal)
    {
        if (pickUpTemplate == null) return;
        Vector3 position = transform.position + boardLocal;
        currentPickUp = Instantiate(pickUpTemplate, position, pickUpTemplate.transform.rotation, pickUpParent);
        currentPickUp.SetActive(true);
    }

    void SpawnEnemy(int row, int col)
    {
        if (enemy == null) return;

        // Nicht direkt neben dem Ball auftauchen lassen
        Vector3 ballLocal = playground.PinPosition - transform.position;
        Vector3 spawnLocal = CellToBoardLocal(row, col, 0f);
        if (Vector2.Distance(new Vector2(spawnLocal.x, spawnLocal.z), new Vector2(ballLocal.x, ballLocal.z)) < minEnemySpawnDistance)
            spawnLocal = RandomFreeCellAwayFrom(ballLocal, minEnemySpawnDistance);

        StartCoroutine(SpawnEnemyAfterWarning(spawnLocal));
    }

    void SpawnEnemyAnywhere()
    {
        if (enemy == null) return;
        Vector3 ballLocal = playground.PinPosition - transform.position;
        StartCoroutine(SpawnEnemyAfterWarning(RandomFreeCellAwayFrom(ballLocal, minEnemySpawnDistance)));
    }

    // Rote, pulsierende Markierung auf dem Brett – danach erscheint dort der Gegner
    IEnumerator SpawnEnemyAfterWarning(Vector3 spawnLocal)
    {
        pendingEnemies++;
        GameObject marker = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        marker.name = "SpawnWarning";
        Destroy(marker.GetComponent<Collider>());
        Renderer markerRenderer = marker.GetComponent<Renderer>();
        markerRenderer.material.color = spawnWarningColor;
        marker.transform.position = transform.position + spawnLocal + Vector3.up * 0.02f;
        marker.transform.SetParent(transform, true); // fährt mit dem Brett mit (nicht unter Ground -> kein NavMesh-Hindernis)

        for (float t = 0f; t < spawnWarningTime; t += Time.deltaTime)
        {
            float pulse = 0.6f + 0.4f * Mathf.Abs(Mathf.Sin(t * 8f));
            marker.transform.localScale = new Vector3(pulse, 0.02f, pulse);
            markerRenderer.enabled = Mathf.Repeat(t, 0.25f) < 0.17f;
            yield return null;
        }
        Destroy(marker);
        pendingEnemies--;

        if (ball == null || ball.HasWon) yield break;
        CreateEnemy(spawnLocal);
    }

    void CreateEnemy(Vector3 spawnLocal)
    {
        // NavMesh-Koordinaten = Brett in Ausgangslage
        Vector3 navPos = playground.StartPosition + spawnLocal;
        if (NavMesh.SamplePosition(navPos, out NavMeshHit hit, 2f, NavMesh.AllAreas))
            navPos = hit.position;

        EnemyMovement clone = Instantiate(enemy, navPos, enemy.transform.rotation);
        clone.name = "Enemy " + (spawnedEnemies.Count + 1);
        clone.gameObject.SetActive(true);
        clone.Teleport(navPos);
        clone.GetComponent<NavMeshAgent>().speed = currentEnemySpeed;
        spawnedEnemies.Add(clone);
    }

    Vector3 FarthestFreeCellFrom(Vector3 boardLocal)
    {
        Vector3 best = Vector3.zero;
        float bestDist = -1f;
        for (int r = 1; r < GridSize - 1; r++)
        {
            for (int c = 1; c < GridSize - 1; c++)
            {
                if (wallGrid[r, c]) continue;
                Vector3 p = CellToBoardLocal(r, c, 0f);
                float d = Vector2.Distance(new Vector2(p.x, p.z), new Vector2(boardLocal.x, boardLocal.z));
                if (d > bestDist) { bestDist = d; best = p; }
            }
        }
        return best;
    }
}
