using System.Collections;
using UnityEngine;
using TMPro;

public class BallGameLogic : MonoBehaviour
{
    public TextMeshProUGUI countText;
    public GameObject winTextObject;

    [Tooltip("Wird vom LevelManager auf die Anzahl der Stufen gesetzt")]
    public int pickUpsToWin = 12;

    [Header("Leben")]
    public int lives = 3;
    [Tooltip("So lange (Sekunden) ist der Ball nach einem Treffer unverwundbar und blinkt")]
    public float invulnerableTime = 2f;

    // Ereignis für LevelManager / Kamera
    public event System.Action Hit;            // Treffer, aber noch Leben übrig

    public bool IsInvulnerable => Time.time < invulnerableUntil;
    public bool HasWon => count >= pickUpsToWin;

    private int count = 0;
    private float invulnerableUntil;
    private Renderer ballRenderer;

    public void SetPickUpsToWin(int total)
    {
        pickUpsToWin = total;
        SetCountText();
    }

    void Start()
    {
        ballRenderer = GetComponent<Renderer>();
        FindMissingUi();
        SetCountText();
        if (winTextObject != null)
            winTextObject.SetActive(false);
    }

    // Falls die Verknüpfungen im Inspector verloren gegangen sind:
    // Texte über ihren Namen in der Szene suchen ("CountText" / "WinText").
    // Sonst bleibt der "You Win!"-Text beim Start sichtbar.
    void FindMissingUi()
    {
        if (countText != null && winTextObject != null) return;

        TextMeshProUGUI[] texts = FindObjectsByType<TextMeshProUGUI>(FindObjectsInactive.Include, FindObjectsSortMode.None);
        foreach (TextMeshProUGUI t in texts)
        {
            if (countText == null && t.name == "CountText") countText = t;
            if (winTextObject == null && t.name == "WinText") winTextObject = t.gameObject;
        }

        if (countText == null || winTextObject == null)
            Debug.LogWarning("BallGameLogic: CountText oder WinText nicht gefunden – bitte im Inspector zuweisen.", this);
    }

    void SetCountText()
    {
        if (countText != null)
            countText.text = "Count: " + count + " / " + pickUpsToWin + "    Leben: " + lives;

        if (HasWon && winTextObject != null)
        {
            winTextObject.SetActive(true);
            winTextObject.GetComponent<TextMeshProUGUI>().text = "You Win!";
        }
    }

    void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("PickUp"))
        {
            other.gameObject.SetActive(false);
            count++;
            SetCountText();
        }
    }

    void OnCollisionEnter(Collision collision)
    {
        // Nur wenn der Ball selbst den Enemy berührt:
        if (collision.gameObject.CompareTag("Enemy"))
            TakeHit();
    }

    // Wird von den Gegnern aufgerufen, sobald sie den Ball erreichen
    public void TakeHit()
    {
        if (HasWon || IsInvulnerable) return;

        lives--;
        SetCountText();

        if (lives > 0)
        {
            Debug.Log("Getroffen! Noch " + lives + " Leben.");
            invulnerableUntil = Time.time + invulnerableTime;
            StartCoroutine(Blink());
            Hit?.Invoke();
            return;
        }

        Debug.Log("Ball wurde gefangen!");
        Destroy(gameObject); // Zerstört nur den Ball
        if (winTextObject != null)
        {
            winTextObject.SetActive(true);
            winTextObject.GetComponent<TextMeshProUGUI>().text = "You Lose!";
        }
    }

    IEnumerator Blink()
    {
        if (ballRenderer == null) yield break;
        while (IsInvulnerable)
        {
            ballRenderer.enabled = !ballRenderer.enabled;
            yield return new WaitForSeconds(0.1f);
        }
        ballRenderer.enabled = true;
    }
}
