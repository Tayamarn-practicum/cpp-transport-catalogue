#pragma once

#include <iostream>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace json_reader {
    void ProcessInput(std::istream& istream, std::ostream& ostream, transport_catalogue::TransportCatalogue& tc);

    void ProcessBaseRequests(json::Node& requests, transport_catalogue::TransportCatalogue& tc);

    void AddStop(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc);

    void AddDist(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc);

    void AddBus(const json::Dict& bus, transport_catalogue::TransportCatalogue& tc);

    void ProcessStatRequests(json::Node& requests_node, transport_catalogue::TransportCatalogue& tc, std::ostream& ostream);

    json::Node ProcessStatRequest(json::Node& request_node, request_handler::RequestHandler& handler);

    void ProcessStopRequest(json::Dict& request_node, json::Dict& responce_node, request_handler::RequestHandler& handler);

    void ProcessBusRequest(json::Dict& request_node, json::Dict& responce_node, request_handler::RequestHandler& handler);

    void ProcessRender(json::Node& requests_node, transport_catalogue::TransportCatalogue& tc, std::ostream& ostream);

    svg::Color GetColor(json::Node& color_node);
}
