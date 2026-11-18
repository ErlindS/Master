using UnityEngine;
using UnityEngine.InputSystem;

// Schräge Verfolger-Kamera, nah am Ball:
// - folgt dem rollenden Ball und schaut etwas in Bewegungsrichtung voraus
// - lehnt sich etwas zum Kunden, für den man ein fertiges Produkt dabei hat
// - Mausrad (oder Gamepad-Schultertasten) zoomt
// - wackelt kurz, wenn ein Kunde verärgert geht
public class CameraController : MonoBehaviour
{
    public GameObject player;

    [Header("Zoom")]
    [Tooltip("Zoom-Stufe beim Start: 0 = ganz nah, 1 = ganz weit weg")]
    [Range(0f, 1f)] public float startZoom = 0.25f;
    [Tooltip("Höhe / Abstand nach hinten, wenn ganz nah herangezoomt")]
    public float nearHeight = 5f;
    public float nearBack = 4.5f;
    [Tooltip("Höhe / Abstand nach hinten, wenn ganz herausgezoomt (ganzes Spielfeld)")]
    public float farHeight = 15f;
    public float farBack = 8f;
    [Tooltip("Zoom-Schritt pro Mausrad-Raste")]
    public float zoomStep = 0.12f;

    [Header("Vorausschauen")]
    [Tooltip("So weit (Einheiten) schaut die Kamera bei voller Geschwindigkeit in Bewegungsrichtung voraus")]
    public float followLookAhead = 1.5f;
    [Tooltip("Anteil (0..1), um den sich die Kamera zum nächsten Ziel (Kunde) verschiebt")]
    [Range(0f, 1f)] public float targetBias = 0.2f;
    [Tooltip("Maximale Verschiebung Richtung Ziel")]
    public float maxTargetShift = 2f;
    [Tooltip("Wie weich die Kamera nachzieht (größer = träger)")]
    public float followSmoothTime = 0.25f;

    [Header("Wackeln bei Treffer")]
    public float shakeDuration = 0.35f;
    public float shakeStrength = 0.3f;

    private PlaygroundMovement board;
    private LevelManager level;
    private BallGameLogic ball;
    private Vector3 focus;
    private Vector3 focusVelocity;
    private float zoom, zoomTarget;
    private float shakeUntil;

    void Start()
    {
        board = FindAnyObjectByType<PlaygroundMovement>();
        level = FindAnyObjectByType<LevelManager>();
        if (player != null)
        {
            ball = player.GetComponent<BallGameLogic>();
            if (ball != null) ball.Hit += Shake;
        }
        zoom = zoomTarget = startZoom;
        focus = BallPosition();
        ApplyTransform(focus, Vector3.zero);
    }

    void OnDestroy()
    {
        if (ball != null) ball.Hit -= Shake;
    }

    public void Shake()
    {
        shakeUntil = Time.time + shakeDuration;
    }

    Vector3 BallPosition()
    {
        if (player != null) return player.transform.position;
        return board != null ? board.PinPosition : transform.position;
    }

    void LateUpdate()
    {
        // Zoom: Mausrad hoch = näher ran
        if (Mouse.current != null)
        {
            float wheel = Mouse.current.scroll.ReadValue().y;
            if (wheel != 0f) zoomTarget -= Mathf.Sign(wheel) * zoomStep;
        }
        Gamepad pad = Gamepad.current;
        if (pad != null)
        {
            if (pad.rightShoulder.isPressed) zoomTarget -= Time.deltaTime;
            if (pad.leftShoulder.isPressed) zoomTarget += Time.deltaTime;
        }
        zoomTarget = Mathf.Clamp01(zoomTarget);
        zoom = Mathf.Lerp(zoom, zoomTarget, 10f * Time.deltaTime);

        Vector3 target = BallPosition();

        if (board != null)
        {
            Vector3 dir = board.BallVelocity / Mathf.Max(board.MaxSpeed, 0.01f);
            dir.y = 0f;
            target += Vector3.ClampMagnitude(dir, 1f) * followLookAhead;
        }

        Transform goal = level != null ? level.CurrentTarget : null;
        if (goal != null)
        {
            Vector3 toGoal = goal.position - BallPosition();
            toGoal.y = 0f;
            target += Vector3.ClampMagnitude(toGoal * targetBias, maxTargetShift);
        }

        focus = Vector3.SmoothDamp(focus, target, ref focusVelocity, followSmoothTime);

        Vector3 shake = Vector3.zero;
        if (Time.time < shakeUntil)
        {
            float k = (shakeUntil - Time.time) / shakeDuration;
            shake = Random.insideUnitSphere * shakeStrength * k;
        }
        ApplyTransform(focus, shake);
    }

    void ApplyTransform(Vector3 lookAt, Vector3 shake)
    {
        float h = Mathf.Lerp(nearHeight, farHeight, zoom);
        float back = Mathf.Lerp(nearBack, farBack, zoom);
        transform.position = lookAt + new Vector3(0f, h, -back) + shake;
        transform.rotation = Quaternion.LookRotation(lookAt - (transform.position - shake), Vector3.up);
    }
}
