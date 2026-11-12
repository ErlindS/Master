using UnityEngine;

// Schräge Verfolger-Kamera für das Schiebebrett:
// - schaut leicht von hinten/oben (man sieht Wände und Gegner besser als von senkrecht oben)
// - schaut in Bewegungsrichtung voraus (dorthin, wo der Ball auf dem Brett hinrollt)
// - lehnt sich etwas zum aktuellen Pickup, damit man weiß, wohin es geht
// - wackelt kurz, wenn der Ball getroffen wird
public class CameraController : MonoBehaviour
{
    public GameObject player;

    [Header("Blickwinkel")]
    [Tooltip("Höhe der Kamera über dem Fokuspunkt")]
    public float height = 15f;
    [Tooltip("Abstand nach hinten (größer = flacher/schräger)")]
    public float distanceBack = 8f;

    [Header("Vorausschauen")]
    [Tooltip("So weit (Einheiten) schaut die Kamera bei voller Geschwindigkeit in Bewegungsrichtung voraus")]
    public float lookAhead = 3f;
    [Tooltip("Anteil (0..1), um den sich die Kamera zum aktuellen Pickup verschiebt")]
    [Range(0f, 1f)] public float pickUpBias = 0.3f;
    [Tooltip("Maximale Verschiebung Richtung Pickup")]
    public float maxPickUpShift = 4f;
    [Tooltip("Wie weich die Kamera nachzieht (größer = träger)")]
    public float smoothTime = 0.4f;

    [Header("Wackeln bei Treffer")]
    public float shakeDuration = 0.35f;
    public float shakeStrength = 0.4f;

    private PlaygroundMovement board;
    private LevelManager level;
    private BallGameLogic ball;
    private Vector3 pin;
    private Vector3 lastBoardPosition;
    private Vector3 boardVelocity;
    private Vector3 focus;
    private Vector3 focusVelocity;
    private float shakeUntil;

    void Start()
    {
        board = FindAnyObjectByType<PlaygroundMovement>();
        level = FindAnyObjectByType<LevelManager>();
        if (player != null)
        {
            pin = player.transform.position;
            ball = player.GetComponent<BallGameLogic>();
            if (ball != null) ball.Hit += Shake;
        }
        if (board != null) lastBoardPosition = board.transform.position;

        focus = pin;
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

    void LateUpdate()
    {
        // Der Ball steht fest; ist er zerstört (Game Over), bleibt der Punkt gleich
        Vector3 target = player != null ? player.transform.position : pin;

        if (board != null && Time.deltaTime > 0f)
        {
            Vector3 v = (board.transform.position - lastBoardPosition) / Time.deltaTime;
            boardVelocity = Vector3.Lerp(boardVelocity, v, 8f * Time.deltaTime);
            lastBoardPosition = board.transform.position;

            // Der Ball rollt relativ zum Brett entgegengesetzt zur Brett-Bewegung
            Vector3 ballDirection = -boardVelocity / Mathf.Max(board.maxSpeed, 0.01f);
            ballDirection.y = 0f;
            target += Vector3.ClampMagnitude(ballDirection, 1f) * lookAhead;
        }

        Transform pickUp = level != null ? level.CurrentPickUp : null;
        if (pickUp != null)
        {
            Vector3 toPickUp = pickUp.position - pin;
            toPickUp.y = 0f;
            target += Vector3.ClampMagnitude(toPickUp * pickUpBias, maxPickUpShift);
        }

        focus = Vector3.SmoothDamp(focus, target, ref focusVelocity, smoothTime);

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
        transform.position = lookAt + new Vector3(0f, height, -distanceBack) + shake;
        transform.rotation = Quaternion.LookRotation(lookAt - (transform.position - shake), Vector3.up);
    }
}
