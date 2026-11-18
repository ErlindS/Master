using System.Text.RegularExpressions;

namespace eShop.AppHost;

/// <summary>
/// Versuchsaufbau Projektarbeit: Fehlerinjektion F1–F7.
/// </summary>
/// <remarks>
/// <list type="bullet">
/// <item>Toxiproxy sitzt zwischen den Diensten und PostgreSQL (F1) bzw. Redis (F6).
/// Die Dienste bekommen dafür Connection-Strings, die auf den Toxiproxy-Port statt auf die Datenbank zeigen.
/// Toxiproxy läuft in allen Läufen mit (auch ohne aktiven Fehler), damit der Aufbau über alle Messungen gleich bleibt.</item>
/// <item>Alle Dienste laden zusätzlich <c>fault-injection/state/faults.json</c> (F4, F5, F7) mit reloadOnChange.
/// Die Datei wird beim Start des AppHost auf "kein Fehler" zurückgesetzt.</item>
/// </list>
/// Gesteuert wird alles über <c>fault-injection/fault.py</c>.
/// </remarks>
internal static class FaultInjection
{
    public const int ToxiproxyApiPort = 8474;

    public static IResourceBuilder<ContainerResource> AddToxiproxy(
        this IDistributedApplicationBuilder builder,
        IResourceBuilder<PostgresServerResource> postgres,
        IResourceBuilder<RedisResource> redis)
    {
        return builder.AddContainer("toxiproxy", "ghcr.io/shopify/toxiproxy", "2.12.0")
            // Fester Port, damit das Fehlersteuerungs-Skript die API ohne Aspire-Abfrage findet.
            .WithHttpEndpoint(port: ToxiproxyApiPort, targetPort: 8474, name: "api", isProxied: false)
            .WithEndpoint(targetPort: 15432, name: "postgres", scheme: "tcp")
            .WithEndpoint(targetPort: 16379, name: "redis", scheme: "tcp")
            .WithBindMount("../../fault-injection/toxiproxy.json", "/config/toxiproxy.json", isReadOnly: true)
            .WithArgs("-host=0.0.0.0", "-config=/config/toxiproxy.json")
            .WaitFor(postgres)
            .WaitFor(redis);
    }

    /// <summary>
    /// Wie <c>WithReference(db)</c>, aber der Connection-String zeigt auf den Toxiproxy-Endpunkt.
    /// Passwort, Datenbankname und ggf. TLS-Einstellungen bleiben unverändert, nur der Port wird ersetzt.
    /// </summary>
    public static IResourceBuilder<T> WithReferenceViaToxiproxy<T, TTarget>(
        this IResourceBuilder<T> project,
        IResourceBuilder<TTarget> target,
        EndpointReference originalEndpoint,
        IResourceBuilder<ContainerResource>? toxiproxy,
        string proxyEndpointName)
        where T : IResourceWithEnvironment, IResourceWithWaitSupport
        where TTarget : class, IResourceWithConnectionString
    {
        project.WithReference(target);

        if (toxiproxy is null)
        {
            return project;
        }

        var proxyEndpoint = toxiproxy.GetEndpoint(proxyEndpointName);
        var value = new ProxiedConnectionString(target.Resource, originalEndpoint, proxyEndpoint);

        return project
            .WithEnvironment(ctx => ctx.EnvironmentVariables[$"ConnectionStrings__{target.Resource.Name}"] = value)
            .WaitFor(toxiproxy);
    }

    /// <summary>
    /// Setzt FAULTS_FILE für alle Projekte und legt die Datei mit "kein Fehler" an.
    /// </summary>
    public static IDistributedApplicationBuilder AddFaultFlags(this IDistributedApplicationBuilder builder)
    {
        var stateDir = Path.GetFullPath(Path.Combine(builder.AppHostDirectory, "..", "..", "fault-injection", "state"));
        var faultsFile = Path.Combine(stateDir, "faults.json");

        if (builder.ExecutionContext.IsRunMode)
        {
            Directory.CreateDirectory(stateDir);
            File.WriteAllText(faultsFile, "{}\n");
        }

        builder.Eventing.Subscribe<BeforeStartEvent>((@event, ct) =>
        {
            foreach (var p in @event.Model.Resources.OfType<Aspire.Hosting.Dotnet.DotnetProjectResource>())
            {
                p.Annotations.Add(new EnvironmentCallbackAnnotation(context =>
                {
                    context.EnvironmentVariables["FAULTS_FILE"] = faultsFile;
                }));
            }

            return Task.CompletedTask;
        });

        return builder;
    }

    private sealed class ProxiedConnectionString(
        IResourceWithConnectionString target,
        EndpointReference original,
        EndpointReference proxy) : IValueProvider, IValueWithReferences, IManifestExpressionProvider
    {
        public string ValueExpression => target.ConnectionStringExpression.ValueExpression;

        public IEnumerable<object> References => [target, original, proxy];

        public ValueTask<string?> GetValueAsync(CancellationToken cancellationToken = default)
            => Rewrite(target.ConnectionStringExpression.GetValueAsync(cancellationToken));

        public ValueTask<string?> GetValueAsync(ValueProviderContext context, CancellationToken cancellationToken = default)
            => Rewrite(target.ConnectionStringExpression.GetValueAsync(context, cancellationToken));

        private async ValueTask<string?> Rewrite(ValueTask<string?> connectionString)
        {
            var value = await connectionString;
            if (value is null)
            {
                return null;
            }

            // Postgres: "Host=localhost;Port=12345;..."; Redis: "localhost:12345,password=..."
            var pattern = $@"(?<=(Port=|:)){original.Port}(?!\d)";
            var rewritten = Regex.Replace(value, pattern, proxy.Port.ToString(), RegexOptions.IgnoreCase);

            if (rewritten == value)
            {
                throw new InvalidOperationException(
                    $"Toxiproxy: Port {original.Port} nicht im Connection-String von '{target.Name}' gefunden.");
            }

            return rewritten;
        }
    }
}
