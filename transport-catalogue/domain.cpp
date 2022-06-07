#include "domain.h"

namespace transport_catalogue {
    Stop::Stop(std::string& name, double lat, double lng) :
        name(name),
        coords({lat, lng})
    {}

    size_t StopPointerPairHasher::operator()(const std::pair<Stop*, Stop*>& obj) const {
        return std::hash<void*>{}(obj.first) + 37 * std::hash<void*>{}(obj.second);
    }

    Bus::Bus(std::string& name,
             std::vector<Stop*>& stops,
             std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists,
             bool is_roundtrip,
             Stop* first, Stop* last) :
        name(name),
        stops(stops),
        is_roundtrip(is_roundtrip),
        first(first),
        last(last)
    {
        std::unordered_set<Stop*> us(stops.begin(), stops.end());
        unique_stops = move(us);
        // We are calculating distances here, because probably input operations will be more frequent, then stat operations.
        true_dist = CalculateTrueDistance(dists);
        geo_dist = CalculateGeoDistance();
    }

    double Bus::CalculateGeoDistance() {
        double res = 0;
        for (size_t i=0; i < stops.size() - 1; ++i) {
            res += ComputeDistance(stops[i]->coords, stops[i+1]->coords);
        }
        return res;
    }

    double Bus::CalculateTrueDistance(std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists) {
        double res = 0;
        for (size_t i=0; i < stops.size() - 1; ++i) {
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
}
