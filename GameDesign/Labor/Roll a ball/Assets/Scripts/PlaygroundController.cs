using UnityEngine;
using UnityEngine.InputSystem;

public class PlaygroundMovement : MonoBehaviour
{
    [Header("Movement Settings")]
    public float moveSpeed = 10f;

    private float movementX;
    private float movementY;

    void OnMove(InputValue movementValue)
    {
        Vector2 movementVector = movementValue.Get<Vector2>();
        movementX = movementVector.x;
        movementY = movementVector.y;
    }

    void Update()
    {
        Vector3 direction = new Vector3(movementX, 0f, movementY);
        transform.position += direction * moveSpeed * Time.deltaTime;
    }
}