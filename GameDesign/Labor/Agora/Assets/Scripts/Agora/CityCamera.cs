using UnityEngine;
using UnityEngine.InputSystem;

// Schräge Draufsicht wie bei Aufbauspielen.
// WASD/Pfeiltasten verschieben, Mausrad zoomt, Q/E drehen.
public class CityCamera : MonoBehaviour
{
    public Vector3 focus = Vector3.zero;
    public float distance = 30f;
    public float minDistance = 8f;
    public float maxDistance = 60f;
    [Range(20f, 89f)] public float pitch = 55f;
    public float yaw = 0f;
    public float panSpeed = 20f;
    public float rotateSpeed = 90f;
    public float zoomSpeed = 0.01f;

    void LateUpdate()
    {
        Keyboard kb = Keyboard.current;
        Mouse mouse = Mouse.current;
        float dt = Time.unscaledDeltaTime; // funktioniert auch in der Pause

        if (kb != null)
        {
            Vector2 move = Vector2.zero;
            if (kb.wKey.isPressed || kb.upArrowKey.isPressed) move.y += 1f;
            if (kb.sKey.isPressed || kb.downArrowKey.isPressed) move.y -= 1f;
            if (kb.dKey.isPressed || kb.rightArrowKey.isPressed) move.x += 1f;
            if (kb.aKey.isPressed || kb.leftArrowKey.isPressed) move.x -= 1f;

            Quaternion yawRot = Quaternion.Euler(0f, yaw, 0f);
            focus += yawRot * new Vector3(move.x, 0f, move.y) * panSpeed * (distance / 30f) * dt;

            if (kb.qKey.isPressed) yaw += rotateSpeed * dt;
            if (kb.eKey.isPressed) yaw -= rotateSpeed * dt;
        }

        if (mouse != null)
        {
            float scroll = mouse.scroll.ReadValue().y;
            distance = Mathf.Clamp(distance - scroll * zoomSpeed * distance, minDistance, maxDistance);
        }

        Quaternion rot = Quaternion.Euler(pitch, yaw, 0f);
        transform.position = focus - rot * Vector3.forward * distance;
        transform.rotation = rot;
    }
}
