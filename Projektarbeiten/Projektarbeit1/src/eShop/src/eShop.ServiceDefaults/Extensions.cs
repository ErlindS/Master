using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Diagnostics.HealthChecks;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Diagnostics.HealthChecks;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using OpenTelemetry.Exporter;
using OpenTelemetry.Logs;
using OpenTelemetry.Metrics;
using OpenTelemetry.Trace;

namespace eShop.ServiceDefaults;

public static partial class Extensions
{
    public static IHostApplicationBuilder AddServiceDefaults(this IHostApplicationBuilder builder)
    {
        builder.AddBasicServiceDefaults();

        builder.Services.AddServiceDiscovery();

        builder.Services.ConfigureHttpClientDefaults(http =>
        {
            // Turn on resilience by default
            http.AddStandardResilienceHandler();

            // Turn on service discovery by default
            http.AddServiceDiscovery();
        });

        return builder;
    }

    /// <summary>
    /// Adds the services except for making outgoing HTTP calls.
    /// </summary>
    /// <remarks>
    /// This allows for things like Polly to be trimmed out of the app if it isn't used.
    /// </remarks>
    public static IHostApplicationBuilder AddBasicServiceDefaults(this IHostApplicationBuilder builder)
    {
        // Fehlerinjektion (Projektarbeit): Feature-Flags F4/F5 und Konfiguration F7 zur Laufzeit umschaltbar.
        // Die Datei wird zuletzt geladen und überschreibt damit appsettings und Umgebungsvariablen.
        var faultsFile = Environment.GetEnvironmentVariable("FAULTS_FILE");
        if (!string.IsNullOrEmpty(faultsFile))
        {
            builder.Configuration.AddJsonFile(faultsFile, optional: true, reloadOnChange: true);
        }

        // Default health checks assume the event bus and self health checks
        builder.AddDefaultHealthChecks();

        builder.ConfigureOpenTelemetry();

        return builder;
    }

    public static IHostApplicationBuilder ConfigureOpenTelemetry(this IHostApplicationBuilder builder)
    {
        builder.Logging.AddOpenTelemetry(logging =>
        {
            logging.IncludeFormattedMessage = true;
            logging.IncludeScopes = true;
        });

        builder.Services.AddOpenTelemetry()
            .WithMetrics(metrics =>
            {
                metrics.AddAspNetCoreInstrumentation()
                    .AddHttpClientInstrumentation()
                    .AddRuntimeInstrumentation()
                    .AddMeter("Experimental.Microsoft.Extensions.AI");
            })
            .WithTracing(tracing =>
            {
                if (builder.Environment.IsDevelopment())
                {
                    // We want to view all traces in development
                    tracing.SetSampler(new AlwaysOnSampler());
                }

                tracing.AddAspNetCoreInstrumentation()
                    .AddGrpcClientInstrumentation()
                    .AddHttpClientInstrumentation()
                    .AddSource("Experimental.Microsoft.Extensions.AI");                    
            });

        builder.AddOpenTelemetryExporters();

        return builder;
    }

    private static IHostApplicationBuilder AddOpenTelemetryExporters(this IHostApplicationBuilder builder)
    {
        var useOtlpExporter = !string.IsNullOrWhiteSpace(builder.Configuration["OTEL_EXPORTER_OTLP_ENDPOINT"]);

        if (useOtlpExporter)
        {
            builder.Services.Configure<OpenTelemetryLoggerOptions>(logging => logging.AddOtlpExporter());
            builder.Services.ConfigureOpenTelemetryMeterProvider(metrics => metrics.AddOtlpExporter());
            builder.Services.ConfigureOpenTelemetryTracerProvider(tracing => tracing.AddOtlpExporter());
        }

        builder.AddGrafanaExporters();

        return builder;
    }

    /// <summary>
    /// Sends logs, traces and metrics additionally to a Grafana backend (e.g. grafana/otel-lgtm or Grafana Cloud).
    /// Enabled when GRAFANA_OTLP_ENDPOINT is set. Optional settings:
    /// GRAFANA_OTLP_PROTOCOL ("grpc" (default) or "http/protobuf") and GRAFANA_OTLP_HEADERS ("key1=value1,key2=value2").
    /// </summary>
    private static IHostApplicationBuilder AddGrafanaExporters(this IHostApplicationBuilder builder)
    {
        var endpoint = builder.Configuration["GRAFANA_OTLP_ENDPOINT"];
        if (string.IsNullOrWhiteSpace(endpoint))
        {
            return builder;
        }

        var protocol = string.Equals(builder.Configuration["GRAFANA_OTLP_PROTOCOL"], "http/protobuf", StringComparison.OrdinalIgnoreCase)
            ? OtlpExportProtocol.HttpProtobuf
            : OtlpExportProtocol.Grpc;

        // Always overwrite the headers, otherwise the Aspire dashboard API key (OTEL_EXPORTER_OTLP_HEADERS) would be sent to Grafana.
        var headers = builder.Configuration["GRAFANA_OTLP_HEADERS"] ?? string.Empty;

        Action<OtlpExporterOptions> Configure(string signalPath) => options =>
        {
            options.Protocol = protocol;
            options.Headers = headers;
            // For http/protobuf the SDK does not append the signal path when the endpoint is set in code.
            options.Endpoint = protocol == OtlpExportProtocol.HttpProtobuf
                ? new Uri($"{endpoint.TrimEnd('/')}/{signalPath}")
                : new Uri(endpoint);
        };

        builder.Services.Configure<OpenTelemetryLoggerOptions>(logging => logging.AddOtlpExporter(Configure("v1/logs")));
        builder.Services.ConfigureOpenTelemetryMeterProvider(metrics => metrics.AddOtlpExporter("grafana", Configure("v1/metrics")));
        builder.Services.ConfigureOpenTelemetryTracerProvider(tracing => tracing.AddOtlpExporter("grafana", Configure("v1/traces")));

        return builder;
    }

    public static IHostApplicationBuilder AddDefaultHealthChecks(this IHostApplicationBuilder builder)
    {
        builder.Services.AddHealthChecks()
            // Add a default liveness check to ensure app is responsive
            .AddCheck("self", () => HealthCheckResult.Healthy(), ["live"]);

        return builder;
    }

    public static WebApplication MapDefaultEndpoints(this WebApplication app)
    {
        // Uncomment the following line to enable the Prometheus endpoint (requires the OpenTelemetry.Exporter.Prometheus.AspNetCore package)
        // app.MapPrometheusScrapingEndpoint();

        // Adding health checks endpoints to applications in non-development environments has security implications.
        // See https://aka.ms/dotnet/aspire/healthchecks for details before enabling these endpoints in non-development environments.
        if (app.Environment.IsDevelopment())
        {
            // All health checks must pass for app to be considered ready to accept traffic after starting
            app.MapHealthChecks("/health");

            // Only health checks tagged with the "live" tag must pass for app to be considered alive
            app.MapHealthChecks("/alive", new HealthCheckOptions
            {
                Predicate = r => r.Tags.Contains("live")
            });
        }

        return app;
    }
}
