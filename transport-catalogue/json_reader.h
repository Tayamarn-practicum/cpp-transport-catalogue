#pragma once

#include <iostream>

#include "graph.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace json_reader {
    struct RoutingSettings {
        int bus_wait_time;
        double bus_velocity;
    };

    void ProcessInput(std::istream& istream, std::ostream& ostream, transport_catalogue::TransportCatalogue& tc);

    void ProcessBaseRequests(json::Node& requests, transport_catalogue::TransportCatalogue& tc, RoutingSettings& routing_settings);

    void AddStop(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc, size_t vertex_id);

    void AddDist(const json::Dict& stop,
                 transport_catalogue::TransportCatalogue& tc,
                 graph::DirectedWeightedGraph<double>& directed_graph,
                 RoutingSettings& routing_settings);

    void AddBus(const json::Dict& bus,
                transport_catalogue::TransportCatalogue& tc,
                graph::DirectedWeightedGraph<double>& directed_graph,
                RoutingSettings& routing_settings);

    void ProcessStatRequests(
        json::Node& requests_node,
        transport_catalogue::TransportCatalogue& tc,
        map_renderer::MapSettings& settings,
        std::ostream& ostream
    );

    json::Node ProcessStatRequest(json::Node& request_node, request_handler::RequestHandler& handler);

    void ProcessStopRequest(json::Dict& request_node, json::Builder& responce_node, request_handler::RequestHandler& handler);

    void ProcessBusRequest(json::Dict& request_node, json::Builder& responce_node, request_handler::RequestHandler& handler);

    void ProcessMapRequest(json::Builder& responce_node, request_handler::RequestHandler& handler);

    map_renderer::MapSettings ProcessRender(json::Node& requests_node);

    RoutingSettings ProcessRouting(json::Node& requests_node);

    svg::Color GetColor(json::Node& color_node);
}
