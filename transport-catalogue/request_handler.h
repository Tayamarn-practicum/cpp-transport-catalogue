// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
#pragma once

#include <optional>
#include <string>
#include <unordered_set>

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
        // MapRenderer понадобится в следующей части итогового проекта
        // RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
        RequestHandler(const transport_catalogue::TransportCatalogue& db);

        // // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через остановку
        const std::unordered_set<transport_catalogue::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        // svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_catalogue::TransportCatalogue& db_;
        // const renderer::MapRenderer& renderer_;
    };
}
