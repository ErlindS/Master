using UnityEngine;

// Ball komplett deaktivieren
public class BallGameLogic : MonoBehaviour
{
    void Awake()
    {
        gameObject.SetActive(false);
    }
}
