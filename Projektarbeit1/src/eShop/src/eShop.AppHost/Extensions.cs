using Aspire.Hosting.Dotnet;
using Aspire.Hosting.Eventing;
using Aspire.Hosting.Foundry;
using Aspire.Hosting.Lifecycle;
using Aspire.Hosting.Yarp;
using Aspire.Hosting.Yarp.Transforms;
using Microsoft.Extensions.Configuration;
using Yarp.ReverseProxy.Configuration;

namespace eShop.AppHost;

internal static class Extensions
{
    public static bool IsFoundryEnabled(IConfiguration configuration) =>
        bool.TryParse(configuration["UseFoundry"], out var useFoundry) && useFoundry;

    /// <summary>
    /// Adds a hook to set the ASPNETCORE_FORWARDEDHEADERS_ENABLED environment variable to true for all projects in the application.
    /// </summary>
    public static IDistributedApplicationBuilder AddForwardedHeaders(this IDistributedApplicationBuilder builder)
    {
        builder.Services.TryAddEventingSubscriber<AddForwardHeadersSubscriber>();
        return builder;
    }

    private class AddForwardHeadersSubscriber : IDistributedApplicationEventingSubscriber
    {
        public Task SubscribeAsync(IDistributedApplicationEventing eventing, DistributedApplicationExecutionContext executionContext, CancellationToken cancellationToken)
        {
            eventing.Subscribe<BeforeStartEvent>((@event, ct) =>
            {
                foreach (var p in @event.Model.Resources.OfType<DotnetProjectResource>())
                {
                    p.Annotations.Add(new EnvironmentCallbackAnnotation(context =>
                    {
                        context.EnvironmentVariables["ASPNETCORE_FORWARDEDHEADERS_ENABLED"] = "true";
                    }));
                }

                return Task.CompletedTask;
            });

            return Task.CompletedTask;
        }
    }

    /// <summary>
    /// Sends logs, traces and metrics of all eShop projects to a Grafana backend (in addition to the Aspire dashboard).
    /// </summary>
    /// <remarks>
    /// By default a local grafana/otel-lgtm container (Grafana, Loki, Tempo, Prometheus and an OpenTelemetry Collector) is started.
    /// To use an external backend such as Grafana Cloud instead, set "Grafana:OtlpEndpoint" (e.g. https://otlp-gateway-prod-eu-west-2.grafana.net/otlp)
    /// and the secret parameter "Parameters:grafana-otlp-headers" (e.g. "Authorization=Basic base64(instanceId:token)") via user secrets.
    /// </remarks>
    public static IDistributedApplicationBuilder AddGrafana(this IDistributedApplicationBuilder builder)
    {
        var externalEndpoint = builder.Configuration["Grafana:OtlpEndpoint"];

        Action<EnvironmentCallbackContext> configureEnvironment;

        if (!string.IsNullOrWhiteSpace(externalEndpoint))
        {
            var headers = builder.AddParameter("grafana-otlp-headers", secret: true);

            configureEnvironment = context =>
            {
                context.EnvironmentVariables["GRAFANA_OTLP_ENDPOINT"] = externalEndpoint;
                context.EnvironmentVariables["GRAFANA_OTLP_PROTOCOL"] = "http/protobuf";
                context.EnvironmentVariables["GRAFANA_OTLP_HEADERS"] = headers.Resource;
            };
        }
        else if (builder.ExecutionContext.IsRunMode)
        {
            var grafana = builder.AddContainer("grafana", "grafana/otel-lgtm", "latest")
                .WithHttpEndpoint(targetPort: 3000, name: "http")
                .WithEndpoint(targetPort: 4317, name: "otlp-grpc", scheme: "http")
                .WithEndpoint(targetPort: 4318, name: "otlp-http", scheme: "http")
                .WithVolume("eshop-grafana-data", "/data")
                // Provision the eShop dashboards (grafana/dashboards/*.json) into the "eShop" folder.
                .WithBindMount("grafana/provisioning/eshop-dashboards.yaml", "/otel-lgtm/grafana/conf/provisioning/dashboards/eshop-dashboards.yaml", isReadOnly: true)
                .WithBindMount("grafana/dashboards", "/otel-lgtm/eshop-dashboards", isReadOnly: true)
                .WithUrlForEndpoint("http", u => u.DisplayText = "Grafana");

            var otlpGrpcEndpoint = grafana.GetEndpoint("otlp-grpc");

            configureEnvironment = context =>
            {
                context.EnvironmentVariables["GRAFANA_OTLP_ENDPOINT"] = otlpGrpcEndpoint;
                context.EnvironmentVariables["GRAFANA_OTLP_PROTOCOL"] = "grpc";
            };
        }
        else
        {
            return builder;
        }

        builder.Eventing.Subscribe<BeforeStartEvent>((@event, ct) =>
        {
            foreach (var project in @event.Model.Resources.OfType<DotnetProjectResource>())
            {
                project.Annotations.Add(new EnvironmentCallbackAnnotation(configureEnvironment));
            }

            return Task.CompletedTask;
        });

        return builder;
    }

    /// <summary>
    /// Configures eShop projects to use Microsoft Foundry for text embedding and chat.
    /// </summary>
    public static IDistributedApplicationBuilder AddFoundry(this IDistributedApplicationBuilder builder,
        IResourceBuilder<DotnetProjectResource> catalogApi,
        IResourceBuilder<DotnetProjectResource> webApp)
    {
        var foundry = builder.AddFoundry("foundry");
        var chat = foundry.AddDeployment("chatModel", "gpt-4.1-mini", "2025-04-14", "OpenAI");
        var textEmbedding = foundry.AddDeployment("textEmbeddingModel", "text-embedding-3-small", "1", "OpenAI");

        catalogApi.WithReference(textEmbedding).WaitFor(textEmbedding);
        webApp.WithReference(chat).WaitFor(chat);

        return builder;
    }

    /// <summary>
    /// Configures eShop projects to use Ollama for text embedding and chat.
    /// </summary>
    public static IDistributedApplicationBuilder AddOllama(this IDistributedApplicationBuilder builder,
        IResourceBuilder<DotnetProjectResource> catalogApi,
        IResourceBuilder<DotnetProjectResource> webApp)
    {
        var ollama = builder.AddOllama("ollama")
            .WithDataVolume()
            .WithGPUSupport()
            .WithOpenWebUI();
        var embeddings = ollama.AddModel("embedding", "all-minilm");
        var chat = ollama.AddModel("chat", "llama3.1");

        catalogApi.WithReference(embeddings)
            .WithEnvironment("OllamaEnabled", "true")
            .WaitFor(embeddings);
        webApp.WithReference(chat)
            .WithEnvironment("OllamaEnabled", "true")
            .WaitFor(chat);

        return builder;
    }

    public static IResourceBuilder<YarpResource> ConfigureMobileBffRoutes(this IResourceBuilder<YarpResource> builder,
        IResourceBuilder<DotnetProjectResource> catalogApi,
        IResourceBuilder<DotnetProjectResource> orderingApi,
        IResourceBuilder<DotnetProjectResource> identityApi)
    {
        return builder.WithConfiguration(yarp =>
        {
            var catalogCluster = yarp.AddCluster(catalogApi);

            yarp.AddRoute("/catalog-api/api/catalog/items", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/by", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/{id}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/by/{name}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/withsemanticrelevance/{text}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/withsemanticrelevance", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/type/{typeId}/brand/{brandId?}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/type/all/brand/{brandId?}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/catalogTypes", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/catalogBrands", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            yarp.AddRoute("/catalog-api/api/catalog/items/{id}/pic", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }])
                .WithTransformPathRemovePrefix("/catalog-api");

            // Generic catalog catch-all route
            yarp.AddRoute("/api/catalog/{*any}", catalogCluster)
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1", "2.0"], Mode = QueryParameterMatchMode.Exact }]);

            // Ordering routes
            yarp.AddRoute("/api/orders/{*any}", orderingApi.GetEndpoint("http"))
                .WithMatchRouteQueryParameter([new() { Name = "api-version", Values = ["1.0", "1"], Mode = QueryParameterMatchMode.Exact }]);

            // Identity routes
            yarp.AddRoute("/identity/{*any}", identityApi.GetEndpoint("http"))
                .WithTransformPathRemovePrefix("/identity");
        });
    }
}
