using UnityEngine;
using UnityEngine.AI;

// Ein Bewohner (früher: Gegner). Folgt einem festen Tagesablauf:
//   Zuhause -> Arbeit (Industrie) -> Agora (Handel) -> Zuhause
// Er handelt nicht "gut" oder "böse", sondern nach den Regeln des Systems:
// Lohn verdienen, Waren kaufen, zufrieden sein oder wegziehen.
public class Resident : MonoBehaviour
{
    enum State { AtHome, ToWork, Working, ToMarket, Shopping, ToHome }

    [Tooltip("Sekunden zu Hause, bevor es wieder losgeht")]
    public float homeSeconds = 2f;
    public float shoppingSeconds = 1f;

    public Building Home { get; private set; }
    public Building Job { get; private set; }
    public bool HasJob => Job != null;
    public float Money => money;
    public float Satisfaction { get; private set; } = 0.5f;

    private CityGrid grid;
    private Economy economy;
    private NavMeshAgent agent;
    private Renderer body;
    private State state = State.AtHome;
    private float timer;
    private float money;
    private float baseSpeed, roadBoost, roadCheckTimer;
    private Building market;

    public void Init(CityGrid owner, Building home, float speed, float roadSpeedBoost, float startMoney)
    {
        grid = owner;
        economy = owner.Economy;
        Home = home;
        money = startMoney;
        body = GetComponent<Renderer>();
        baseSpeed = speed * Random.Range(0.85f, 1.15f);
        roadBoost = roadSpeedBoost;

        agent = gameObject.AddComponent<NavMeshAgent>();
        agent.speed = baseSpeed;
        agent.acceleration = 20f;
        agent.angularSpeed = 540f;
        agent.baseOffset = 1f; // Kapsel-Mittelpunkt liegt eine halbe Höhe über dem Boden
        agent.radius = 0.5f;
        agent.obstacleAvoidanceType = ObstacleAvoidanceType.NoObstacleAvoidance; // dürfen sich überlappen
        agent.Warp(home.transform.position);

        timer = Random.Range(0f, homeSeconds); // nicht alle gleichzeitig loslaufen
        SetVisible(false);
        UpdateColor();
    }

    void Update()
    {
        timer -= Time.deltaTime;
        UpdateRoadSpeed();

        switch (state)
        {
            case State.AtHome:
                if (timer <= 0f) LeaveHome();
                break;

            case State.ToWork:
                if (Job == null) { GoHome(); break; }
                if (Arrived()) { state = State.Working; timer = economy.rules.shiftSeconds; SetVisible(false); }
                break;

            case State.Working:
                if (Job == null) { GoHome(); break; }
                if (timer <= 0f)
                {
                    money += economy.PayShift();
                    GoToMarket();
                }
                break;

            case State.ToMarket:
                if (market == null) { GoToMarket(); break; }
                if (Arrived()) { state = State.Shopping; timer = shoppingSeconds; SetVisible(false); }
                break;

            case State.Shopping:
                if (timer <= 0f)
                {
                    bool bought = economy.TryBuy(ref money);
                    ChangeSatisfaction(bought ? 0.2f : -0.15f);
                    GoHome();
                }
                break;

            case State.ToHome:
                if (Arrived()) ArriveHome();
                break;
        }
    }

    // ---- Tagesablauf ----

    void LeaveHome()
    {
        if (Job == null) TryFindJob();

        if (Job != null)
        {
            state = State.ToWork;
            MoveTo(Job.transform.position);
        }
        else if (money >= economy.Price)
        {
            ChangeSatisfaction(-0.05f); // arbeitslos, kauft vom Ersparten
            GoToMarket();
        }
        else
        {
            ChangeSatisfaction(-0.1f);  // arbeitslos und pleite
            state = State.AtHome;
            timer = homeSeconds * 2f;
        }
    }

    void TryFindJob()
    {
        Building workplace = grid.FindNearest(ZoneKind.Industrie, transform.position, b => b.HasFreeJob);
        if (workplace != null && workplace.TryHire()) Job = workplace;
    }

    void GoToMarket()
    {
        market = grid.FindNearest(ZoneKind.Agora, transform.position);
        if (market == null)
        {
            ChangeSatisfaction(-0.1f); // keine Agora -> kein Handel möglich
            GoHome();
            return;
        }
        state = State.ToMarket;
        MoveTo(market.transform.position);
    }

    void GoHome()
    {
        state = State.ToHome;
        MoveTo(Home.transform.position);
    }

    void ArriveHome()
    {
        if (grid.HasInstitutionNear(Home.transform.position))
            ChangeSatisfaction(economy.rules.institutionBonus);
        state = State.AtHome;
        timer = homeSeconds;
        SetVisible(false);
    }

    // ---- Von außen ----

    public void LoseJob() => Job = null; // Arbeitsplatz wurde abgerissen

    // Haus abgerissen oder geschrumpft: Bewohner verschwindet aus der Stadt
    public void MoveOut()
    {
        if (Job != null) Job.Fire();
        Job = null;
        grid.RemoveResident(this);
        Destroy(gameObject);
    }

    // ---- Hilfen ----

    void UpdateRoadSpeed()
    {
        roadCheckTimer -= Time.deltaTime;
        if (roadCheckTimer > 0f) return;
        roadCheckTimer = 0.2f;
        bool onRoad = NavMesh.SamplePosition(agent.nextPosition, out NavMeshHit _, 0.3f, 1 << CityGrid.RoadArea);
        agent.speed = baseSpeed * (onRoad ? roadBoost : 1f);
    }

    void MoveTo(Vector3 target)
    {
        SetVisible(true);
        Vector2 jitter = Random.insideUnitCircle * grid.cellSize * 0.2f;
        agent.SetDestination(target + new Vector3(jitter.x, 0f, jitter.y));
    }

    bool Arrived() => !agent.pathPending && agent.remainingDistance <= 0.3f;

    void ChangeSatisfaction(float delta)
    {
        Satisfaction = Mathf.Clamp01(Satisfaction + delta);
        UpdateColor();
    }

    void UpdateColor()
    {
        // rot = unzufrieden, grün = zufrieden
        if (body != null)
            body.sharedMaterial = CityMaterials.Get(SatisfactionColor(Mathf.Round(Satisfaction * 10f) / 10f));
    }

    public static Color SatisfactionColor(float value)
    {
        return value < 0.5f
            ? Color.Lerp(new Color(0.85f, 0.2f, 0.15f), new Color(0.95f, 0.8f, 0.2f), value * 2f)
            : Color.Lerp(new Color(0.95f, 0.8f, 0.2f), new Color(0.25f, 0.75f, 0.3f), (value - 0.5f) * 2f);
    }

    void SetVisible(bool visible)
    {
        if (body != null) body.enabled = visible;
    }
}
