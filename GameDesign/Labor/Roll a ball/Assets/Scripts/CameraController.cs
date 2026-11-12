using UnityEngine;
using UnityEngine.InputSystem;

public class CameraController : MonoBehaviour
{
    public float panSpeed = 10f;
    public float zoomSpeed = 6f;

    private Vector3 currentPos;

    void Start()
    {
        // Augenhöhe in der Mailänder Flaniermeile
        currentPos = new Vector3(0f, 3.2f, -9f);
        transform.position = currentPos;
        transform.rotation = Quaternion.Euler(14f, 0f, 0f);
    }

    void Update()
    {
        if (Keyboard.current == null) return;

        Vector3 move = Vector3.zero;
        if (Keyboard.current.wKey.isPressed || Keyboard.current.upArrowKey.isPressed) move.z += 1f;
        if (Keyboard.current.sKey.isPressed || Keyboard.current.downArrowKey.isPressed) move.z -= 1f;
        if (Keyboard.current.aKey.isPressed || Keyboard.current.leftArrowKey.isPressed) move.x -= 1f;
        if (Keyboard.current.dKey.isPressed || Keyboard.current.rightArrowKey.isPressed) move.x += 1f;

        currentPos += move.normalized * panSpeed * Time.deltaTime;

        if (Keyboard.current.qKey.isPressed) currentPos.y += zoomSpeed * Time.deltaTime;
        if (Keyboard.current.eKey.isPressed) currentPos.y -= zoomSpeed * Time.deltaTime;
        currentPos.y = Mathf.Clamp(currentPos.y, 2f, 15f);

        transform.position = Vector3.Lerp(transform.position, currentPos, 10f * Time.deltaTime);
    }
}
