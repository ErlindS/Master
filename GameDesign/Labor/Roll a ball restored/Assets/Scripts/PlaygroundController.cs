using UnityEngine;
using UnityEngine.InputSystem;

// Schlitzbrett: Der Ball hängt fest an einer Stange und bewegt sich nie.
// Der Spieler schiebt das Spielfeld darunter herum. Die Labyrinth-Mauern
// sind die "Schlitze": Stößt eine Mauer gegen den Ball, blockiert das Brett
// in diese Richtung (wie bei einem Schiebelabyrinth mit festem Stift).
[RequireComponent(typeof(Rigidbody))]
public class PlaygroundMovement : MonoBehaviour
{
    [Header("Movement Settings")]
    [Tooltip("Maximale Geschwindigkeit des Bretts")]
    public float maxSpeed = 8f;

    [Tooltip("Zeit in Sekunden, bis das Brett die Zielgeschwindigkeit ungefähr erreicht (kleiner = direkter)")]
    public float smoothTime = 0.08f;

    [Tooltip("An: Der Ball rollt in die gedrückte Richtung (das Brett fährt dafür entgegengesetzt).\n" +
             "Aus: Das Brett fährt in die gedrückte Richtung.")]
    public bool invertControls = true;

    [Header("Ball / Stift")]
    [Tooltip("Radius für die Kollision Ball <-> Mauern (etwas kleiner als der echte Ball, damit er nicht an Ecken hängen bleibt)")]
    public float ballCollisionRadius = 0.45f;

    [Tooltip("Ball dreht sich mit, als würde er im Schlitz rollen")]
    public bool spinBall = true;

    [Header("Physik")]
    [Tooltip("Physik-Schritte pro Sekunde")]
    public int physicsRate = 100;

    private Rigidbody rb;
    private Rigidbody ballRb;
    private LevelManager level;
    private Vector3 startPosition;
    private Vector3 pinPosition;
    private Vector3 velocity;
    private Vector3 velocitySmoothing; // Hilfsvariable für SmoothDamp
    private Vector2 input;

    // Ausgangsposition des Bretts (dort liegt auch das NavMesh)
    public Vector3 StartPosition => startPosition;

    // Fester Punkt in der Welt, an dem der Ball hängt
    public Vector3 PinPosition => pinPosition;

    void Awake()
    {
        rb = GetComponent<Rigidbody>();
        rb.isKinematic = true;
        rb.constraints = RigidbodyConstraints.None; // im Inspector war "alles einfrieren" gesetzt
        rb.interpolation = RigidbodyInterpolation.Interpolate;
        startPosition = transform.position;
        level = GetComponent<LevelManager>();

        if (physicsRate > 0)
            Time.fixedDeltaTime = 1f / physicsRate;

        BallGameLogic ball = FindAnyObjectByType<BallGameLogic>();
        if (ball != null)
        {
            pinPosition = ball.transform.position;
            ballRb = ball.GetComponent<Rigidbody>();
            if (ballRb != null)
            {
                // Ball festhalten: keine Schwerkraft, Position eingefroren.
                // Er bleibt aber ein dynamischer Körper, damit Pickups (Trigger)
                // und der Gegner (Kollision) weiterhin erkannt werden.
                ballRb.useGravity = false;
                ballRb.linearVelocity = Vector3.zero;
                ballRb.constraints = RigidbodyConstraints.FreezePosition;
                ballRb.sleepThreshold = 0f; // nie einschlafen, sonst werden Kontakte verpasst
            }
        }
        else
        {
            pinPosition = startPosition + Vector3.up * 0.5f;
        }
    }

    // Setzt das Brett sofort zurück in die Ausgangsposition
    public void ResetBoard()
    {
        SetBoardPosition(startPosition);
    }

    // Schiebt das Brett so, dass der angegebene Punkt (relativ zum Brett) unter dem Ball liegt
    public void CenterBoardOn(Vector3 boardLocalPoint)
    {
        Vector3 pos = pinPosition - boardLocalPoint;
        pos.y = startPosition.y;
        SetBoardPosition(pos);
    }

    // Führt eine Aktion aus, während das Brett kurz in seiner Ausgangslage steht
    // (z. B. NavMesh backen), und stellt danach die Position wieder her.
    // Die Geschwindigkeit bleibt erhalten, die Bewegung des Spielers stockt also nicht.
    public void RunWithBoardAtStart(System.Action action)
    {
        Vector3 current = rb.position;
        transform.position = startPosition;
        rb.position = startPosition;
        Physics.SyncTransforms();
        try { action(); }
        finally
        {
            transform.position = current;
            rb.position = current;
            Physics.SyncTransforms();
        }
    }

    void SetBoardPosition(Vector3 position)
    {
        velocity = Vector3.zero;
        velocitySmoothing = Vector3.zero;
        transform.position = position;
        rb.position = position;
        Physics.SyncTransforms();
    }

    // Kommt über die PlayerInput-Komponente (Send Messages)
    void OnMove(InputValue movementValue)
    {
        input = Vector2.ClampMagnitude(movementValue.Get<Vector2>(), 1f);
    }

    // Liest Tastatur/Gamepad zusätzlich direkt aus. So funktioniert die Steuerung
    // auch dann, wenn die PlayerInput-Komponente kein Gerät bekommt (z. B. weil
    // ein zweiter PlayerInput die Tastatur schon belegt hat).
    Vector2 ReadDevicesDirectly()
    {
        Vector2 v = Vector2.zero;
        Keyboard kb = Keyboard.current;
        if (kb != null)
        {
            if (kb.aKey.isPressed || kb.leftArrowKey.isPressed) v.x -= 1f;
            if (kb.dKey.isPressed || kb.rightArrowKey.isPressed) v.x += 1f;
            if (kb.sKey.isPressed || kb.downArrowKey.isPressed) v.y -= 1f;
            if (kb.wKey.isPressed || kb.upArrowKey.isPressed) v.y += 1f;
        }
        Gamepad pad = Gamepad.current;
        if (pad != null)
        {
            Vector2 stick = pad.leftStick.ReadValue();
            if (stick.sqrMagnitude > v.sqrMagnitude) v = stick;
        }
        return Vector2.ClampMagnitude(v, 1f);
    }

    void FixedUpdate()
    {
        float dt = Time.fixedDeltaTime;

        // Stärkere der beiden Quellen nehmen (sie liefern normalerweise dasselbe)
        Vector2 direct = ReadDevicesDirectly();
        Vector2 move = direct.sqrMagnitude > input.sqrMagnitude ? direct : input;

        // Invertiert: Das Brett fährt entgegen der Eingabe -> der Ball rollt
        // auf dem Brett genau in die gedrückte Richtung.
        if (invertControls) move = -move;

        Vector3 targetVelocity = new Vector3(move.x, 0f, move.y) * maxSpeed;
        velocity = Vector3.SmoothDamp(velocity, targetVelocity, ref velocitySmoothing, smoothTime, Mathf.Infinity, dt);

        // Achsen getrennt prüfen, damit das Brett an Mauern entlanggleiten kann
        Vector3 pos = rb.position;
        Vector3 step = velocity * dt;

        Vector3 tryX = pos + new Vector3(step.x, 0f, 0f);
        if (IsBlocked(tryX)) velocity.x = 0f; else pos = tryX;

        Vector3 tryZ = pos + new Vector3(0f, 0f, step.z);
        if (IsBlocked(tryZ)) velocity.z = 0f; else pos = tryZ;

        rb.MovePosition(pos);

        // Ball rollt relativ zum Brett in die Gegenrichtung -> passend mitdrehen
        if (spinBall && ballRb != null)
        {
            Vector3 ballVelocityOnBoard = -velocity;
            float radius = ballRb.transform.lossyScale.x * 0.5f;
            ballRb.angularVelocity = Vector3.Cross(Vector3.up, ballVelocityOnBoard) / Mathf.Max(radius, 0.01f);
        }
    }

    // Würde der Ball bei dieser Brett-Position in einer Mauer stecken?
    bool IsBlocked(Vector3 boardPosition)
    {
        if (level == null) return false;
        Vector3 ballOnBoard = pinPosition - boardPosition;
        return level.IsBlocked(ballOnBoard, ballCollisionRadius);
    }
}
