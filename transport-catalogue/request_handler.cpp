#include "request_handler.h"

#include <sstream>

#include <iostream>


namespace request_handler {
    BusStat::BusStat(double curvature, double route_length, int stop_count, int unique_stop_count) :
        curvature(curvature),
        route_length(route_length),
        stop_count(stop_count),
        unique_stop_count(unique_stop_count)
    {}

    RequestHandler::RequestHandler(
        const transport_catalogue::TransportCatalogue& db,
        const map_renderer::MapRenderer& renderer,
        const graph::DirectedWeightedGraph<double>& directed_graph,
        const graph::Router<double>& router
    ) :
        db_(db),
        map_renderer_(renderer),
        directed_graph_(directed_graph),
        router_(router)
    {}

    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        auto busname_to_bus = db_.GetBusnames();
        if (busname_to_bus.find(bus_name) == busname_to_bus.end()) {
            return std::nullopt;
        }
        transport_catalogue::Bus* bus = busname_to_bus.at(bus_name);
        BusStat bus_stat {bus->GetCurvature(),
                          bus->GetTrueDistance(),
                          static_cast<int>(bus->stops.size()),
                          static_cast<int>(bus->unique_stops.size())};
        return bus_stat;
    }

    const std::unordered_set<transport_catalogue::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        auto stopname_to_stop = db_.GetStopnames();
        if (stopname_to_stop.find(stop_name) == stopname_to_stop.end()) {
            return nullptr;
        }
        transport_catalogue::Stop* stop = stopname_to_stop.at(stop_name);
        return db_.GetBusesByStop(stop);
    }

    std::string RequestHandler::RenderMap() const {
        std::ostringstream sstream;
        map_renderer_.Render(
            db_.GetBusnamesPtr(),
            db_.GetStopnamesPtr(),
            sstream
        );
        return sstream.str();
    }

    std::optional<graph::Router<double>::RouteInfo> RequestHandler::RouteInfo(const std::string_view from_stop_name, const std::string_view to_stop_name) const {
        auto stopname_to_stop = db_.GetStopnames();
        if (stopname_to_stop.find(from_stop_name) == stopname_to_stop.end()) {
            return {};
        }
        if (stopname_to_stop.find(to_stop_name) == stopname_to_stop.end()) {
            return {};
        }
        transport_catalogue::Stop* from_stop = stopname_to_stop.at(from_stop_name);
        transport_catalogue::Stop* to_stop = stopname_to_stop.at(to_stop_name);
        size_t start_vortex = from_stop->in_vertex;
        size_t end_vortex = to_stop->in_vertex;
        return router_.BuildRoute(start_vortex, end_vortex);
    }

    graph::Edge<double> RequestHandler::GraphEdgeInfo(graph::EdgeId edge_id) const {
        return directed_graph_.GetEdge(edge_id);
    }

    transport_catalogue::Stop* RequestHandler::StopByVertex(size_t vertex_id) const {
        return db_.GetVertexToStops()->at(vertex_id);
    }

    std::pair<transport_catalogue::Bus*, int> RequestHandler::BusSpanByEdge(size_t edge_id) const {
        return db_.GetEdgeSpanToBuses()->at(edge_id);
    }
}
