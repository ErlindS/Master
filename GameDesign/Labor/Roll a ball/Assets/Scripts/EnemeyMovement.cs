using UnityEngine;
using UnityEngine.AI;

public class EnemyMovement : MonoBehaviour
{
    // Reference to the player's transform.
    public Transform player;

    // Das bewegliche Spielfeld (wird automatisch gesucht, falls leer).
    public Transform board;

    [Tooltip("Abstand (waagerecht, Mitte zu Mitte), ab dem der Ball als gefangen gilt")]
    public float catchDistance = 0.85f;

    [Tooltip("Wie weit (in Sekunden) der Gegner vorausdenkt und dem Ball den Weg abschneidet. 0 = direkt hinterher")]
    public float maxLeadTime = 0.35f;

    private BallGameLogic ball;
    private Vector3 lastBoardPosition;
    private Vector3 boardVelocity;

    private NavMeshAgent navMeshAgent;
    private Vector3 boardStartPosition;

    // Awake statt Start: Der LevelManager klont Gegner und platziert sie
    // sofort danach mit Teleport() – dafür muss alles schon eingerichtet sein.
    void Awake()
    {
        navMeshAgent = GetComponent<NavMeshAgent>();

        // Verfolger-Einstellungen: nicht vor dem Ziel abbremsen,
        // schnell beschleunigen und wenden, bis ganz ran an den Ball.
        navMeshAgent.autoBraking = false;
        navMeshAgent.stoppingDistance = 0f;
        navMeshAgent.acceleration = 60f;
        navMeshAgent.angularSpeed = 720f;

        if (player != null) ball = player.GetComponent<BallGameLogic>();

        PlaygroundMovement playground = board != null
            ? board.GetComponent<PlaygroundMovement>()
            : FindAnyObjectByType<PlaygroundMovement>();

        if (playground != null)
        {
            board = playground.transform;
            // Startposition vom Brett holen (nicht die aktuelle Position),
            // weil der Gegner erst später im Spiel erzeugt werden kann.
            boardStartPosition = playground.StartPosition;
            // Der Agent rechnet weiter auf dem (festen) NavMesh,
            // die sichtbare Position setzen wir selbst.
            navMeshAgent.updatePosition = false;
            lastBoardPosition = board.position;
        }
        else
        {
            board = null;
        }
    }

    // Setzt den Gegner auf einen Punkt des NavMesh (NavMesh-Koordinaten,
    // also relativ zum Brett in Ausgangslage) und passt die sichtbare Position an.
    public void Teleport(Vector3 navMeshPosition)
    {
        navMeshAgent.Warp(navMeshPosition);
        if (board != null)
            transform.position = navMeshAgent.nextPosition + (board.position - boardStartPosition);
    }

    void Update()
    {
        if (board == null)
        {
            // Fallback: altes Verhalten
            if (player != null && navMeshAgent.isOnNavMesh)
                navMeshAgent.SetDestination(player.position);
            CheckCatch();
            return;
        }

        // Das NavMesh liegt fest an der Startposition des Bretts.
        // Wir rechnen alles in "Brett-Koordinaten" um, damit der Gegner
        // mit dem Spielfeld mitfährt und trotzdem den Ball verfolgt.
        Vector3 boardDelta = board.position - boardStartPosition;

        // Geschwindigkeit des Bretts (geglättet). Der Ball bewegt sich relativ
        // zum Brett genau entgegengesetzt.
        if (Time.deltaTime > 0f)
        {
            Vector3 v = (board.position - lastBoardPosition) / Time.deltaTime;
            boardVelocity = Vector3.Lerp(boardVelocity, v, 10f * Time.deltaTime);
        }
        lastBoardPosition = board.position;

        if (player != null && navMeshAgent.isOnNavMesh)
            navMeshAgent.SetDestination(PredictTarget(player.position - boardDelta));

        transform.position = navMeshAgent.nextPosition + boardDelta;

        CheckCatch();
    }

    // Zielpunkt: dort, wo der Ball gleich sein wird (Weg abschneiden).
    // Je näher der Gegner ist, desto weniger rechnet er voraus.
    Vector3 PredictTarget(Vector3 ballOnNavMesh)
    {
        Vector3 ballVelocity = -boardVelocity;
        ballVelocity.y = 0f;

        float distance = Vector3.Distance(navMeshAgent.nextPosition, ballOnNavMesh);
        float lead = Mathf.Min(distance / Mathf.Max(navMeshAgent.speed, 0.1f), maxLeadTime);
        if (distance < 1.5f || lead <= 0f) return ballOnNavMesh;

        Vector3 predicted = ballOnNavMesh + ballVelocity * lead;
        if (NavMesh.SamplePosition(predicted, out NavMeshHit hit, 2f, NavMesh.AllAreas))
            return hit.position;
        return ballOnNavMesh;
    }

    // Fangen per Abstand statt per Physik-Kollision: Der Gegner wird über
    // transform.position bewegt und hat keinen Rigidbody, daher ist
    // OnCollisionEnter hier unzuverlässig.
    void CheckCatch()
    {
        if (player == null || ball == null) return;

        Vector3 diff = player.position - transform.position;
        diff.y = 0f;
        if (diff.sqrMagnitude < catchDistance * catchDistance)
            ball.TakeHit();
    }
}
