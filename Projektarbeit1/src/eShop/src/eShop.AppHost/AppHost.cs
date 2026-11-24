using System.IO.Pipes;
using eShop.AppHost;

var builder = DistributedApplication.CreateBuilder(args);

builder.AddForwardedHeaders();
builder.AddGrafana();
builder.AddAzureContainerAppEnvironment("aca");

var redis = builder.AddRedis("redis");
var rabbitMq = builder.AddRabbitMQ("eventbus")
    .WithLifetime(ContainerLifetime.Persistent);
var postgres = builder.AddPostgres("postgres")
    .WithImage("ankane/pgvector")
    .WithImageTag("latest")
    .WithLifetime(ContainerLifetime.Persistent);

var catalogDb = postgres.AddDatabase("catalogdb");
var identityDb = postgres.AddDatabase("identitydb");
var orderDb = postgres.AddDatabase("orderingdb");
var webhooksDb = postgres.AddDatabase("webhooksdb");

// Fehlerinjektion (Projektarbeit): Toxiproxy vor PostgreSQL (F1) und Redis (F6), Feature-Flags (F4, F5, F7)
builder.AddFaultFlags();
var toxiproxy = builder.ExecutionContext.IsRunMode ? builder.AddToxiproxy(postgres, redis) : null;
var pgEndpoint = postgres.Resource.PrimaryEndpoint;
var redisEndpoint = redis.Resource.PrimaryEndpoint;

var launchProfileName = ShouldUseHttpForEndpoints() ? "http" : "https";

// Services
var identityApi = builder.AddDotnetProject("identity-api", "../Identity.API", o => o.LaunchProfileName = launchProfileName)
    .WithExternalHttpEndpoints()
    .WithReferenceViaToxiproxy(identityDb, pgEndpoint, toxiproxy, "postgres")
    .WithHttpHealthCheck("/health");

var identityEndpoint = identityApi.GetEndpoint(launchProfileName);
    
var basketApi = builder.AddDotnetProject("basket-api", "../Basket.API")
    .WithReferenceViaToxiproxy(redis, redisEndpoint, toxiproxy, "redis")
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WithEnvironment("Identity__Url", identityEndpoint);
redis.WithParentRelationship(basketApi);

var catalogApi = builder.AddDotnetProject("catalog-api", "../Catalog.API")
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WithReferenceViaToxiproxy(catalogDb, pgEndpoint, toxiproxy, "postgres");

var orderingApi = builder.AddDotnetProject("ordering-api", "../Ordering.API")
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WithReferenceViaToxiproxy(orderDb, pgEndpoint, toxiproxy, "postgres").WaitFor(orderDb)
    .WithHttpHealthCheck("/health")
    .WithEnvironment("Identity__Url", identityEndpoint);

builder.AddDotnetProject("order-processor", "../OrderProcessor")
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WithReferenceViaToxiproxy(orderDb, pgEndpoint, toxiproxy, "postgres")
    .WaitFor(orderingApi); // wait for the orderingApi to be ready because that contains the EF migrations
    
builder.AddDotnetProject("payment-processor", "../PaymentProcessor")
    .WithReference(rabbitMq).WaitFor(rabbitMq);

var webHooksApi = builder.AddDotnetProject("webhooks-api", "../Webhooks.API")
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WithReferenceViaToxiproxy(webhooksDb, pgEndpoint, toxiproxy, "postgres")
    .WithEnvironment("Identity__Url", identityEndpoint);

// Reverse proxies
builder.AddYarp("mobile-bff")
    .WithExternalHttpEndpoints()
    .ConfigureMobileBffRoutes(catalogApi, orderingApi, identityApi);

// Apps
var webhooksClient = builder.AddDotnetProject("webhooksclient", "../WebhookClient", o => o.LaunchProfileName = launchProfileName)
    .WithReference(webHooksApi)
    .WithEnvironment("IdentityUrl", identityEndpoint);

var webApp = builder.AddDotnetProject("webapp", "../WebApp", o => o.LaunchProfileName = launchProfileName)
    .WithExternalHttpEndpoints()
    .WithUrls(c => c.Urls.ForEach(u => u.DisplayText = $"Online Store ({u.Endpoint?.EndpointName})"))
    .WithReference(basketApi)
    .WithReference(catalogApi)
    .WithReference(orderingApi)
    .WithReference(rabbitMq).WaitFor(rabbitMq)
    .WaitFor(identityApi)
    .WithEnvironment("IdentityUrl", identityEndpoint);

// Set UseFoundry=true to provision Microsoft Foundry for chat and embeddings.
bool useFoundry = Extensions.IsFoundryEnabled(builder.Configuration);
if (useFoundry)
{
    builder.AddFoundry(catalogApi, webApp);
}

bool useOllama = false;
if (useOllama)
{
    builder.AddOllama(catalogApi, webApp);
}

// Wire up the callback urls (self referencing)
webApp.WithEnvironment("CallBackUrl", webApp.GetEndpoint(launchProfileName));
webhooksClient.WithEnvironment("CallBackUrl", webhooksClient.GetEndpoint(launchProfileName));

// Identity has a reference to all of the apps for callback urls, this is a cyclic reference
identityApi.WithEnvironment("BasketApiClient", basketApi.GetEndpoint("http"))
           .WithEnvironment("OrderingApiClient", orderingApi.GetEndpoint("http"))
           .WithEnvironment("WebhooksApiClient", webHooksApi.GetEndpoint("http"))
           .WithEnvironment("WebhooksWebClient", webhooksClient.GetEndpoint(launchProfileName))
           .WithEnvironment("WebAppClient", webApp.GetEndpoint(launchProfileName));

builder.Build().Run();

// For test use only.
// Looks for an environment variable that forces the use of HTTP for all the endpoints. We
// are doing this for ease of running the Playwright tests in CI.
static bool ShouldUseHttpForEndpoints()
{
    const string EnvVarName = "ESHOP_USE_HTTP_ENDPOINTS";
    var envValue = Environment.GetEnvironmentVariable(EnvVarName);

    // Attempt to parse the environment variable value; return true if it's exactly "1".
    return int.TryParse(envValue, out int result) && result == 1;
}
