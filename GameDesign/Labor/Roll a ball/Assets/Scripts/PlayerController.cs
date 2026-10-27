using UnityEngine;
using TMPro;

public class BallGameLogic : MonoBehaviour
{
    public TextMeshProUGUI countText;
    public GameObject winTextObject;

    private int count = 0;

    void Start()
    {
        SetCountText();
        if (winTextObject != null)
            winTextObject.SetActive(false);
    }

    void SetCountText()
    {
        if (countText != null)
            countText.text = "Count: " + count.ToString();

        if (count >= 12 && winTextObject != null)
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
        {
            Destroy(gameObject); // Zerstört nur den Ball
            if (winTextObject != null)
            {
                winTextObject.SetActive(true);
                winTextObject.GetComponent<TextMeshProUGUI>().text = "You Lose!";
            }
        }
    }
}