using UnityEngine;
using UnityEngine.InputSystem;

[RequireComponent(typeof(Rigidbody))]
public class PlaygroundMovement : MonoBehaviour
{
    public float speed = 10f;
    private Rigidbody rb;
    private Vector2 input;

    void Awake()
    {
        rb = GetComponent<Rigidbody>();
        rb.isKinematic = true;
    }

    void OnMove(InputValue val)
    {
        input = val.Get<Vector2>();
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
        Vector3 move = new Vector3(input.x, 0f, input.y) * speed * Time.fixedDeltaTime;
        rb.MovePosition(rb.position + move);
    }
}
