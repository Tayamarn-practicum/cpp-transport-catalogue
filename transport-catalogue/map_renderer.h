#pragma once

#include <deque>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "svg.h"
#include "domain.h"

namespace map_renderer {

    struct MapSettings {
        MapSettings(
            double width,
            double height,
            double padding,
            double line_width,
            double stop_radius,
            int bus_label_font_size,
            std::pair<double, double> bus_label_offset,
            int stop_label_font_size,
            std::pair<double, double> stop_label_offset,
            svg::Color& underlayer_color,
            double underlayer_width,
            std::vector<svg::Color>& color_palette
        );

        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        std::pair<double, double> bus_label_offset;
        int stop_label_font_size;
        std::pair<double, double> stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    class ZoomCoefProcessor {
    public:
        ZoomCoefProcessor(MapSettings& settings, std::unordered_set<transport_catalogue::Stop*>& stops_in_routes);

        double GetCoef() const;

        svg::Point CoordsToPoint(geo::Coordinates coords) const;
    private:
        double min_lat_;
        double min_lng_;
        double max_lat_;
        double max_lng_;
        double coef_;
        double padding_;
    };

    void RenderMap(
        std::map<std::string_view, transport_catalogue::Bus*>* buses,
        MapSettings& settings,
        std::ostream& ostream
    );
}
