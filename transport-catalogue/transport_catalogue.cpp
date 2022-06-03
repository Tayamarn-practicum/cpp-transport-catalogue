#include "transport_catalogue.h"

#include <iostream>

// Comment this out because practicum's platform doesn't support custom files.
// #include "log_duration.h"

namespace transport_catalogue {
    Stop::Stop(std::string& name, double lat, double lng) :
        name(name),
        coords({lat, lng})
    {}

    Bus::Bus(std::string& name,
             std::vector<Stop*>& stops,
             std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists) :
        name(name),
        stops(stops)
    {
        // LOG_DURATION("BusCreation");
        std::set<Stop*> us(stops.begin(), stops.end());
        unique_stops = move(us);
        // We are calculating distances here, because probably input operations will be more frequent, then stat operations.
        true_dist = CalculateTrueDistance(dists);
        geo_dist = CalculateGeoDistance();
    }

    double Bus::CalculateGeoDistance() {
        // LOG_DURATION("CalculateGeoDistance");
        double res = 0;
        for (int i=0; i < stops.size() - 1; ++i) {
            res += ComputeDistance(stops[i]->coords, stops[i+1]->coords);
        }
        return res;
    }

    double Bus::CalculateTrueDistance(std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists) {
        // LOG_DURATION("CalculateTrueDistance");
        double res = 0;
        // auto dists = tc.GetDists();
        for (int i=0; i < stops.size() - 1; ++i) {
            auto itr = dists->find({stops[i], stops[i+1]});
            if (itr != dists->end()) {
                res += itr->second;
            } else {
                res += dists->at({stops[i+1], stops[i]});
            }
        }
        return res;
    }

    double Bus::GetCurvature() {
        return GetTrueDistance() / GetGeoDistance();
    }

    double Bus::GetGeoDistance() {
        return geo_dist;
    }

    double Bus::GetTrueDistance() {
        return true_dist;
    }

    size_t StopPointerPairHasher::operator()(const std::pair<Stop*, Stop*>& obj) const {
        return std::hash<void*>{}(obj.first) + 37 * std::hash<void*>{}(obj.second);
    }

    TransportCatalogue::TransportCatalogue() {}

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        Stop* current_stop = &(stops_[stops_.size() - 1]);
        stopname_to_stop_[current_stop->name] = current_stop;
        stop_to_buses_[current_stop] = {};
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        Bus* current_bus = &(buses_[buses_.size() - 1]);
        busname_to_bus_[current_bus->name] = current_bus;
        for (Stop* s : bus.stops) {
            stop_to_buses_.at(s).insert(current_bus);
        }
    }

    void TransportCatalogue::AddDistance(Stop* s1, Stop* s2, int dist) {
        dist_between_stops_[{s1, s2}] = dist;
    }

    std::deque<Stop> TransportCatalogue::GetStops() {
        return stops_;
    }

    std::map<std::string_view, Stop*> TransportCatalogue::GetStopnames() {
        return stopname_to_stop_;
    }

    Stop* TransportCatalogue::StopByName(const std::string& stop_name) {
        return stopname_to_stop_.at(stop_name);
    }

    std::deque<Bus> TransportCatalogue::GetBuses() {
        return buses_;
    }

    std::map<std::string_view, Bus*> TransportCatalogue::GetBusnames() {
        return busname_to_bus_;
    }

    std::unordered_set<Bus*> TransportCatalogue::GetBusesByStop(Stop* stop) {
        return stop_to_buses_.at(stop);
    }

    std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* TransportCatalogue::GetDists() {
        return &dist_between_stops_;
    }
}
