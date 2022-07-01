// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include "graph.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"

namespace request_handler {

    struct BusStat {
        BusStat(double curvature, double route_length, int stop_count, int unique_stop_count);

        double curvature;
        double route_length;
        int stop_count;
        int unique_stop_count;
    };

    class RequestHandler {
    public:
        RequestHandler(
            const transport_catalogue::TransportCatalogue& db,
            const map_renderer::MapRenderer& renderer,
            const graph::DirectedWeightedGraph<double>& directed_graph,
            const graph::Router<double>& router
        );

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку
        const std::unordered_set<transport_catalogue::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        std::string RenderMap() const;

        std::optional<graph::Router<double>::RouteInfo> RouteInfo(const std::string_view from_stop_name, const std::string_view to_stop_name) const;
        graph::Edge<double> GraphEdgeInfo(graph::EdgeId edge_id) const;
        transport_catalogue::Stop* StopByVertex(size_t vertex_id) const;
        std::pair<transport_catalogue::Bus*, int> BusSpanByEdge(size_t edge_id) const;

    private:
        const transport_catalogue::TransportCatalogue& db_;
        const map_renderer::MapRenderer& map_renderer_;
        const graph::DirectedWeightedGraph<double>& directed_graph_;
        const graph::Router<double>& router_;
    };
}
