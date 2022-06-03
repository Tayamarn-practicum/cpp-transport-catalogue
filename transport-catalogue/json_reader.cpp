#include "json_reader.h"

#include <algorithm>
#include <string>
#include <vector>

namespace json_reader {
    void ProcessInput(std::istream& istream, std::ostream& ostream, transport_catalogue::TransportCatalogue& tc) {
        const auto doc = json::Load(istream);
        json::Dict root = doc.GetRoot().AsMap();
        ProcessBaseRequests(root.at("base_requests"), tc);
        ProcessStatRequests(root.at("stat_requests"), tc, ostream);
    }

    void ProcessBaseRequests(json::Node& requests_node, transport_catalogue::TransportCatalogue& tc) {
        json::Array requests = requests_node.AsArray();
        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Stop") {
                AddStop(map, tc);
            }
        }
        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Stop") {
                AddDist(map, tc);
            }
        }
        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Bus") {
                AddBus(map, tc);
            }
        }
    }

    void AddStop(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc) {
        std::string name = stop.at("name").AsString();
        tc.AddStop({name,
                    stop.at("latitude").AsDouble(),
                    stop.at("longitude").AsDouble()});
    }

    void AddDist(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc) {
        auto dists = stop.at("road_distances").AsMap();
        std::string name = stop.at("name").AsString();
        auto this_stop = tc.StopByName(name);
        for (auto [to_name, dist_node] : dists) {
            auto to_stop = tc.StopByName(to_name);
            tc.AddDistance(this_stop, to_stop, dist_node.AsInt());
        }
    }

    void AddBus(const json::Dict& bus, transport_catalogue::TransportCatalogue& tc) {
        std::string name = bus.at("name").AsString();
        std::vector<transport_catalogue::Stop*> stops;

        auto stop_names = bus.at("stops").AsArray();
        for (auto stop_name : stop_names) {
            stops.push_back(tc.StopByName(stop_name.AsString()));
        }

        if (!bus.at("is_roundtrip").AsBool()) {
            for (int i=stop_names.size()-2;i>=0;--i) {
                stops.push_back(tc.StopByName(stop_names[i].AsString()));
            }
        }
        tc.AddBus({name, stops, tc.GetDists()});
    }

    void ProcessStatRequests(json::Node& requests_node, transport_catalogue::TransportCatalogue& tc, std::ostream& ostream) {
        json::Array requests = requests_node.AsArray();
        json::Array responces;
        request_handler::RequestHandler handler(tc);
        for (auto req : requests) {
            responces.push_back(ProcessStatRequest(req, handler));
        }
        json::Print(json::Document{responces}, ostream);
    }

    json::Node ProcessStatRequest(json::Node& request_node, request_handler::RequestHandler& handler) {
        auto request = request_node.AsMap();
        json::Dict resp;
        resp["request_id"] = request.at("id").AsInt();
        if (request.at("type").AsString() == "Stop") {
            ProcessStopRequest(request, resp, handler);
        } else if (request.at("type").AsString() == "Bus") {
            ProcessBusRequest(request, resp, handler);
        }
        return resp;
    }

    void ProcessStopRequest(json::Dict& request_node, json::Dict& responce_node, request_handler::RequestHandler& handler) {
        const std::unordered_set<transport_catalogue::Bus*>* buses = handler.GetBusesByStop(request_node.at("name").AsString());
        if (buses == nullptr) {
            json::Node node{static_cast<std::string>("not found")};
            responce_node["error_message"] = node;
            return;
        }
        std::vector<std::string> bus_names;
        for (transport_catalogue::Bus* bus : *buses) {
            bus_names.push_back(bus->name);
        }
        std::sort(bus_names.begin(), bus_names.end());
        json::Array bus_nodes;
        for (auto bus : bus_names) {
            bus_nodes.push_back(bus);
        }
        responce_node["buses"] = bus_nodes;
    }

    void ProcessBusRequest(json::Dict& request_node, json::Dict& responce_node, request_handler::RequestHandler& handler) {
        auto bus_stat = handler.GetBusStat(request_node.at("name").AsString());
        if (!bus_stat) {
            json::Node node{static_cast<std::string>("not found")};
            responce_node["error_message"] = node;
            return;
        }
        responce_node["curvature"] = bus_stat->curvature;
        responce_node["route_length"] = bus_stat->route_length;
        responce_node["stop_count"] = bus_stat->stop_count;
        responce_node["unique_stop_count"] = bus_stat->unique_stop_count;
    }
}
