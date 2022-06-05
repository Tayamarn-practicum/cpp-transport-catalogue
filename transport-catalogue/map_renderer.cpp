#include "map_renderer.h"

#include <algorithm>
#include <unordered_set>

#include <iostream>

namespace map_renderer {
        MapSettings::MapSettings(
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
        ) :
            width(width),
            height(height),
            padding(padding),
            line_width(line_width),
            stop_radius(stop_radius),
            bus_label_font_size(bus_label_font_size),
            bus_label_offset(bus_label_offset),
            stop_label_font_size(stop_label_font_size),
            stop_label_offset(stop_label_offset),
            underlayer_color(underlayer_color),
            underlayer_width(underlayer_width),
            color_palette(color_palette)
        {}

    ZoomCoefProcessor::ZoomCoefProcessor(MapSettings& settings, std::unordered_set<transport_catalogue::Stop*>& stops_in_routes) :
        padding_(settings.padding)
    {
        min_lat_ = std::numeric_limits<double>::infinity();
        min_lng_ = std::numeric_limits<double>::infinity();
        max_lat_ = std::numeric_limits<double>::lowest();
        max_lng_ = std::numeric_limits<double>::lowest();
        for (auto stop : stops_in_routes) {
            if (stop->coords.lat > max_lat_) max_lat_ = stop->coords.lat;
            if (stop->coords.lat < min_lat_) min_lat_ = stop->coords.lat;
            if (stop->coords.lng > max_lng_) max_lng_ = stop->coords.lng;
            if (stop->coords.lng < min_lng_) min_lng_ = stop->coords.lng;
        }
        if ((max_lng_ == min_lng_) && (max_lat_ == min_lat_)) {
            coef_ = 0;
            return;
        }
        double width_zoom_coef = std::numeric_limits<double>::infinity();
        double height_zoom_coef = std::numeric_limits<double>::infinity();
        if (max_lng_ != min_lng_) {
            width_zoom_coef = (settings.width - 2 * settings.padding) / (max_lng_ - min_lng_);
        }
        if (max_lat_ == min_lat_) {
            height_zoom_coef = (settings.height - 2 * settings.padding) / (max_lat_ - min_lat_);
        }
        coef_ = std::min(width_zoom_coef, height_zoom_coef);
    }

    double ZoomCoefProcessor::GetCoef() const {
        return coef_;
    }

    svg::Point ZoomCoefProcessor::CoordsToPoint(geo::Coordinates coords) const {
        double x = (coords.lng - min_lng_) * coef_ + padding_;
        double y = (max_lat_ - coords.lat) * coef_ + padding_;
        return {x, y};
    }

    void RenderMap(
        std::map<std::string_view, transport_catalogue::Bus*>* buses,
        MapSettings& settings,
        std::ostream& ostream
    ) {
        std::vector<std::string_view> bus_names;
        std::unordered_set<transport_catalogue::Stop*> stops_in_routes;
        for (auto [name, bus] : *buses) {
            bus_names.push_back(name);
            stops_in_routes.merge(bus->unique_stops);
        }
        std::sort(bus_names.begin(), bus_names.end());

        ZoomCoefProcessor zoom_coef_proc {settings, stops_in_routes};

        svg::Document doc;
        for (size_t i=0; i<bus_names.size();++i) {
            int color_num = i % settings.color_palette.size();
            svg::Color bus_color = settings.color_palette[color_num];
            transport_catalogue::Bus* bus = buses->at(bus_names[i]);
            svg::Polyline poly;
            poly.SetStrokeColor(bus_color)
                .SetStrokeWidth(settings.line_width)
                .SetFillColor("none")
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            for (transport_catalogue::Stop* stop : bus->stops) {
                poly.AddPoint(zoom_coef_proc.CoordsToPoint(stop->coords));
            }
            doc.Add(poly);
        }
        doc.Render(ostream);
    }

}
