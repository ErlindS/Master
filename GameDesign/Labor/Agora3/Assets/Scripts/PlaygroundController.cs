using UnityEngine;
using UnityEngine.InputSystem;

// Normale Ball-Steuerung: Das Spielfeld steht fest, der Ball rollt mit
// WASD / Pfeiltasten / Gamepad darüber. Kollision mit den Mauern über das
// Raster des LevelManagers (Achsen getrennt, damit der Ball an Mauern entlanggleitet).
// Bleibt aus Kompatibilitätsgründen auf dem Spielfeld-Objekt ("Walls").
[RequireComponent(typeof(Rigidbody))]
public class PlaygroundMovement : MonoBehaviour
{
    [Header("Ball-Bewegung")]
    [Tooltip("Höchstgeschwindigkeit des Balls")]
    public float ballSpeed = 7f;

    [Tooltip("Zeit in Sekunden, bis der Ball die Zielgeschwindigkeit ungefähr erreicht (kleiner = direkter)")]
    public float ballSmoothTime = 0.1f;

    [Tooltip("Radius für die Kollision Ball <-> Mauern (etwas kleiner als der echte Ball, damit er nicht an Ecken hängen bleibt)")]
    public float ballCollisionRadius = 0.45f;

    [Tooltip("Ball dreht sich beim Rollen mit")]
    public bool spinBall = true;

    [Header("Physik")]
    [Tooltip("Physik-Schritte pro Sekunde")]
    public int physicsRate = 100;

    private Rigidbody boardRb;
    private Rigidbody ballRb;
    private Transform ballTransform;
    private LevelManager level;
    private Vector3 startPosition;
    private Vector3 ballPosition;
    private Vector3 velocity;
    private Vector3 velocitySmoothing;
    private Vector2 input;

    // Position des Spielfelds (steht fest, dort liegt auch das NavMesh)
    public Vector3 StartPosition => startPosition;

    // Aktuelle Position des Balls in der Welt
    public Vector3 PinPosition => ballPosition;

    // Aktuelle Geschwindigkeit des Balls (für die Kamera)
    public Vector3 BallVelocity => velocity;
    public float MaxSpeed => ballSpeed;

    void Awake()
    {
        boardRb = GetComponent<Rigidbody>();
        boardRb.isKinematic = true;
        startPosition = transform.position;
        level = GetComponent<LevelManager>();

        if (physicsRate > 0)
            Time.fixedDeltaTime = 1f / physicsRate;

        BallGameLogic ball = FindAnyObjectByType<BallGameLogic>();
        if (ball != null)
        {
            ballTransform = ball.transform;
            ballPosition = ballTransform.position;
            ballRb = ball.GetComponent<Rigidbody>();
            if (ballRb != null)
            {
                // Wir bewegen den Ball selbst (kinematisch), die Mauern prüft das Raster
                ballRb.isKinematic = true;
                ballRb.useGravity = false;
                ballRb.constraints = RigidbodyConstraints.None;
                ballRb.interpolation = RigidbodyInterpolation.Interpolate;
            }
        }
        else
        {
            ballPosition = startPosition + Vector3.up * 0.5f;
        }
    }

    // Setzt den Ball auf den angegebenen Punkt (relativ zum Spielfeld)
    public void CenterBoardOn(Vector3 boardLocalPoint)
    {
        Vector3 pos = startPosition + boardLocalPoint;
        pos.y = ballPosition.y;
        ballPosition = pos;
        velocity = Vector3.zero;
        velocitySmoothing = Vector3.zero;
        if (ballRb != null) ballRb.position = pos;
        if (ballTransform != null) ballTransform.position = pos;
        Physics.SyncTransforms();
    }

    // Das Spielfeld steht ohnehin in seiner Ausgangslage (z. B. für das NavMesh-Backen)
    public void RunWithBoardAtStart(System.Action action) => action();

    // Kommt über die PlayerInput-Komponente (Send Messages)
    void OnMove(InputValue movementValue)
    {
        input = Vector2.ClampMagnitude(movementValue.Get<Vector2>(), 1f);
    }

    // Liest Tastatur/Gamepad zusätzlich direkt aus (falls PlayerInput kein Gerät bekommt)
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
        if (ballTransform == null) return;
        float dt = Time.fixedDeltaTime;

        Vector2 direct = ReadDevicesDirectly();
        Vector2 move = direct.sqrMagnitude > input.sqrMagnitude ? direct : input;

        Vector3 targetVelocity = new Vector3(move.x, 0f, move.y) * ballSpeed;
        velocity = Vector3.SmoothDamp(velocity, targetVelocity, ref velocitySmoothing, ballSmoothTime, Mathf.Infinity, dt);

        // Achsen getrennt prüfen, damit der Ball an Mauern entlanggleiten kann
        Vector3 pos = ballPosition;
        Vector3 step = velocity * dt;

        Vector3 tryX = pos + new Vector3(step.x, 0f, 0f);
        if (IsBlocked(tryX)) velocity.x = 0f; else pos = tryX;

        Vector3 tryZ = pos + new Vector3(0f, 0f, step.z);
        if (IsBlocked(tryZ)) velocity.z = 0f; else pos = tryZ;

        ballPosition = pos;
        if (ballRb != null) ballRb.MovePosition(pos);
        else ballTransform.position = pos;

        // Rollen: Drehachse senkrecht zur Bewegungsrichtung
        if (spinBall && velocity.sqrMagnitude > 0.0001f)
        {
            float radius = Mathf.Max(ballTransform.lossyScale.x * 0.5f, 0.01f);
            Vector3 axis = Vector3.Cross(Vector3.up, velocity.normalized);
            float degrees = velocity.magnitude * dt / radius * Mathf.Rad2Deg;
            Quaternion spin = Quaternion.AngleAxis(degrees, axis) * ballTransform.rotation;
            if (ballRb != null) ballRb.MoveRotation(spin);
            else ballTransform.rotation = spin;
        }
    }

    // Würde der Ball an dieser Weltposition in einer Mauer stecken?
    bool IsBlocked(Vector3 worldBallPosition)
    {
        if (level == null) return false;
        return level.IsBlocked(worldBallPosition - transform.position, ballCollisionRadius);
    }
}
