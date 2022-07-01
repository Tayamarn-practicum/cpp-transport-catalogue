#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <iostream>

namespace json_reader {
    void ProcessInput(std::istream& istream, std::ostream& ostream, transport_catalogue::TransportCatalogue& tc) {
        const auto doc = json::Load(istream);
        json::Dict root = doc.GetRoot().AsMap();
        auto map_settings = ProcessRender(root.at("render_settings"));
        auto routing_settings = ProcessRouting(root.at("routing_settings"));
        auto directed_graph = ProcessBaseRequests(root.at("base_requests"), tc, routing_settings);
        ProcessStatRequests(root.at("stat_requests"), tc, map_settings, ostream, directed_graph);
    }

    graph::DirectedWeightedGraph<double> ProcessBaseRequests(
        json::Node& requests_node,
        transport_catalogue::TransportCatalogue& tc,
        RoutingSettings& routing_settings
    ) {
        json::Array requests = requests_node.AsArray();
        size_t vertex_id = 0;
        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Stop") {
                AddStop(map, tc, vertex_id);
                vertex_id += 2;
            }
        }

        graph::DirectedWeightedGraph<double> directed_graph(vertex_id);

        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Stop") {
                AddDist(map, tc, directed_graph, routing_settings);
            }
        }
        for (auto req : requests) {
            auto map = req.AsMap();
            if (map.at("type").AsString() == "Bus") {
                AddBus(map, tc, directed_graph, routing_settings);
            }
        }
        return directed_graph;
    }

    void AddStop(const json::Dict& stop, transport_catalogue::TransportCatalogue& tc, size_t vertex_id) {
        std::string name = stop.at("name").AsString();
        tc.AddStop({name,
                    stop.at("latitude").AsDouble(),
                    stop.at("longitude").AsDouble(),
                    vertex_id});
    }

    void AddDist(
        const json::Dict& stop,
        transport_catalogue::TransportCatalogue& tc,
        graph::DirectedWeightedGraph<double>& directed_graph,
        RoutingSettings& routing_settings
    ) {
        auto dists = stop.at("road_distances").AsMap();
        std::string name = stop.at("name").AsString();
        auto this_stop = tc.StopByName(name);
        directed_graph.AddEdge({this_stop->in_vertex, this_stop->out_vertex, static_cast<double>(routing_settings.bus_wait_time)});
        for (auto [to_name, dist_node] : dists) {
            auto to_stop = tc.StopByName(to_name);
            tc.AddDistance(this_stop, to_stop, dist_node.AsInt());
        }
    }

    void AddBus(
        const json::Dict& bus,
        transport_catalogue::TransportCatalogue& tc,
        graph::DirectedWeightedGraph<double>& directed_graph,
        RoutingSettings& routing_settings
    ) {
        std::string name = bus.at("name").AsString();
        std::vector<transport_catalogue::Stop*> stops;

        auto stop_names = bus.at("stops").AsArray();
        for (auto stop_name : stop_names) {
            stops.push_back(tc.StopByName(stop_name.AsString()));
        }
        transport_catalogue::Stop* first = stops[0];
        transport_catalogue::Stop* last = stops[stops.size() - 1];

        if (!bus.at("is_roundtrip").AsBool()) {
            for (int i=stop_names.size()-2;i>=0;--i) {
                stops.push_back(tc.StopByName(stop_names[i].AsString()));
            }
        }
        auto tc_bus = tc.AddBus({name, stops, tc.GetDists(), bus.at("is_roundtrip").AsBool(), first, last});

        // Add edges to the graph
        for (auto slow_it = stops.begin(); slow_it != stops.end(); ++slow_it) {
            int total_dist = 0;
            for (auto fast_it = next(slow_it); fast_it != stops.end(); ++fast_it) {
                auto itr = tc.GetDists()->find({*prev(fast_it), *fast_it});
                if (itr != tc.GetDists()->end()) {
                    total_dist += itr->second;
                } else {
                    total_dist += tc.GetDists()->at({*fast_it, *prev(fast_it)});
                }
                auto edge = directed_graph.AddEdge({
                    (*slow_it)->out_vertex,
                    (*fast_it)->in_vertex,
                    total_dist / routing_settings.bus_velocity
                });
                tc.AddEdgeSpanToBus(edge, tc_bus, (fast_it - slow_it));
            }
        }
    }

    void ProcessStatRequests(
        json::Node& requests_node,
        transport_catalogue::TransportCatalogue& tc,
        map_renderer::MapSettings& map_settings,
        std::ostream& ostream,
        graph::DirectedWeightedGraph<double>& directed_graph
    ) {
        json::Array requests = requests_node.AsArray();
        json::Builder responces;
        responces.StartArray();
        map_renderer::MapRenderer mr {map_settings};
        graph::Router router(directed_graph);
        request_handler::RequestHandler handler(tc, mr, directed_graph, router);
        for (auto req : requests) {
            responces.Value(ProcessStatRequest(req, handler).GetValue());
        }
        responces.EndArray();
        json::Print(json::Document{responces.Build()}, ostream);
    }

    json::Node ProcessStatRequest(json::Node& request_node, request_handler::RequestHandler& handler) {
        auto request = request_node.AsMap();
        json::Builder resp;
        resp.StartDict();
        resp.Key(static_cast<std::string>("request_id")).Value(request.at("id").AsInt());
        std::string req_type = request.at("type").AsString();
        if (req_type == "Stop") {
            ProcessStopRequest(request, resp, handler);
        } else if (req_type == "Bus") {
            ProcessBusRequest(request, resp, handler);
        } else if (req_type == "Map") {
            ProcessMapRequest(resp, handler);
        } else if (req_type == "Route") {
            ProcessRouteRequest(request, resp, handler);
        }

        resp.EndDict();
        return resp.Build();
    }

    void ProcessStopRequest(json::Dict& request_node, json::Builder& responce_node, request_handler::RequestHandler& handler) {
        const std::unordered_set<transport_catalogue::Bus*>* buses = handler.GetBusesByStop(request_node.at("name").AsString());
        if (buses == nullptr) {
            responce_node.Key(static_cast<std::string>("error_message")).Value(static_cast<std::string>("not found"));
            return;
        }
        std::vector<std::string> bus_names;
        for (transport_catalogue::Bus* bus : *buses) {
            bus_names.push_back(bus->name);
        }
        std::sort(bus_names.begin(), bus_names.end());
        responce_node.Key(static_cast<std::string>("buses")).StartArray();
        for (auto bus : bus_names) {
            responce_node.Value(bus);
        }
        responce_node.EndArray();
    }

    void ProcessBusRequest(json::Dict& request_node, json::Builder& responce_node, request_handler::RequestHandler& handler) {
        auto bus_stat = handler.GetBusStat(request_node.at("name").AsString());
        if (!bus_stat) {
            responce_node.Key(static_cast<std::string>("error_message")).Value(static_cast<std::string>("not found"));
            return;
        }
        responce_node.Key(static_cast<std::string>("curvature")).Value(bus_stat->curvature);
        responce_node.Key(static_cast<std::string>("route_length")).Value(bus_stat->route_length);
        responce_node.Key(static_cast<std::string>("stop_count")).Value(bus_stat->stop_count);
        responce_node.Key(static_cast<std::string>("unique_stop_count")).Value(bus_stat->unique_stop_count);
    }

    void ProcessMapRequest(json::Builder& responce_node, request_handler::RequestHandler& handler) {
        responce_node.Key(static_cast<std::string>("map")).Value(handler.RenderMap());
    }

    void ProcessRouteRequest(json::Dict& request_node, json::Builder& responce_node, request_handler::RequestHandler& handler) {
        auto route_info = handler.RouteInfo(request_node.at("from").AsString(), request_node.at("to").AsString());
        if (!route_info.has_value()) {
            responce_node.Key(static_cast<std::string>("error_message")).Value(static_cast<std::string>("not found"));
            return;
        }
        responce_node.Key(static_cast<std::string>("total_time")).Value(route_info->weight);

        responce_node.Key(static_cast<std::string>("items")).StartArray();
        for (auto edge_id : route_info->edges) {
            responce_node.StartDict();
            auto edge = handler.GraphEdgeInfo(edge_id);
            responce_node.Key(static_cast<std::string>("time")).Value(edge.weight);
            auto from_stop = handler.StopByVertex(edge.from);
            auto to_stop = handler.StopByVertex(edge.to);
            if (from_stop == to_stop) {
                responce_node.Key(static_cast<std::string>("type")).Value(static_cast<std::string>("Wait"));
                responce_node.Key(static_cast<std::string>("stop_name")).Value(from_stop->name);
            } else {
                responce_node.Key(static_cast<std::string>("type")).Value(static_cast<std::string>("Bus"));
                auto [bus, span] = handler.BusSpanByEdge(edge_id);
                responce_node.Key(static_cast<std::string>("bus")).Value(bus->name);
                responce_node.Key(static_cast<std::string>("span_count")).Value(span);
            }
            responce_node.EndDict();
        }
        responce_node.EndArray();
    }

    map_renderer::MapSettings ProcessRender(json::Node& requests_node) {
        json::Dict request = requests_node.AsMap();
        svg::Color underlayer_color = GetColor(request.at("underlayer_color"));
        std::vector<svg::Color> color_palette;
        for (json::Node node : request.at("color_palette").AsArray()) {
            color_palette.push_back(GetColor(node));
        }
        map_renderer::MapSettings settings{
            request.at("width").AsDouble(),
            request.at("height").AsDouble(),
            request.at("padding").AsDouble(),
            request.at("line_width").AsDouble(),
            request.at("stop_radius").AsDouble(),
            request.at("bus_label_font_size").AsInt(),
            {
                request.at("bus_label_offset").AsArray()[0].AsDouble(),
                request.at("bus_label_offset").AsArray()[1].AsDouble()
            },
            request.at("stop_label_font_size").AsInt(),
            {
                request.at("stop_label_offset").AsArray()[0].AsDouble(),
                request.at("stop_label_offset").AsArray()[1].AsDouble()
            },
            underlayer_color,
            request.at("underlayer_width").AsDouble(),
            color_palette
        };
        return settings;
    }

    RoutingSettings ProcessRouting(json::Node& requests_node) {
        json::Dict request = requests_node.AsMap();
        return {
            request.at("bus_wait_time").AsInt(),
            request.at("bus_velocity").AsDouble() * 1000 / 60. // We want meters per minute
        };
    }

    svg::Color GetColor(json::Node& color_node) {
        if (color_node.IsString()) {
            return color_node.AsString();
        }
        if (color_node.IsArray()) {
            json::Array color_arr = color_node.AsArray();
            if (color_arr.size() == 3) {
                svg::Rgb rgb {
                    static_cast<uint8_t>(color_arr[0].AsInt()),
                    static_cast<uint8_t>(color_arr[1].AsInt()),
                    static_cast<uint8_t>(color_arr[2].AsInt())
                };
                return rgb;
            }
            if (color_arr.size() == 4) {
                svg::Rgba rgba {
                    static_cast<uint8_t>(color_arr[0].AsInt()),
                    static_cast<uint8_t>(color_arr[1].AsInt()),
                    static_cast<uint8_t>(color_arr[2].AsInt()),
                    color_arr[3].AsDouble()
                };
                return rgba;
            }
        }
        return {};
    }
}
