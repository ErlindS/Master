using UnityEngine;

// Dreht Schriften (und Sprechblasen) so, dass sie zur Kamera zeigen
public class FaceCamera : MonoBehaviour
{
    void LateUpdate()
    {
        Camera cam = Camera.main;
        if (cam != null) transform.rotation = cam.transform.rotation;
    }
}
