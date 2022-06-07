// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include "map_renderer.h"
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
        RequestHandler(const transport_catalogue::TransportCatalogue& db, const map_renderer::MapSettings& settings);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку
        const std::unordered_set<transport_catalogue::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        std::string RenderMap() const;

    private:
        const transport_catalogue::TransportCatalogue& db_;
        const map_renderer::MapSettings& map_settings_;
    };
}
