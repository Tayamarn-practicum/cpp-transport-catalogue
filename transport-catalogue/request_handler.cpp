#include "request_handler.h"

#include <sstream>


namespace request_handler {
    BusStat::BusStat(double curvature, double route_length, int stop_count, int unique_stop_count) :
        curvature(curvature),
        route_length(route_length),
        stop_count(stop_count),
        unique_stop_count(unique_stop_count)
    {}

    RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const map_renderer::MapRenderer& renderer) :
        db_(db),
        map_renderer_(renderer)
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
}
