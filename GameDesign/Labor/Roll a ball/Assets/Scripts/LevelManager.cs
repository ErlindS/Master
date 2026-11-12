using UnityEngine;
public class LevelManager : MonoBehaviour
{
    void Awake()
    {
        gameObject.AddComponent<MarketCityManager>();
    }
}
