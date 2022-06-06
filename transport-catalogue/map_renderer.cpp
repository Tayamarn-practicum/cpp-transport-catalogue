#include "map_renderer.h"

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

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    double SphereProjector::GetCoef() const {
        return zoom_coeff_;
    }

    void RenderMap(
        std::map<std::string_view, transport_catalogue::Bus*>* buses,
        std::map<std::string_view, transport_catalogue::Stop*>* stops,
        MapSettings& settings,
        std::ostream& ostream
    ) {
        std::vector<std::string_view> bus_names;
        std::vector<std::string_view> stop_names;
        std::unordered_set<transport_catalogue::Stop*> stops_in_routes;
        for (auto [name, bus] : *buses) {
            bus_names.push_back(name);
            stops_in_routes.merge(bus->unique_stops);
        }
        std::sort(bus_names.begin(), bus_names.end());
        for (auto stop : stops_in_routes) {
            stop_names.push_back(stop->name);
        }
        std::sort(bus_names.begin(), bus_names.end());
        std::sort(stop_names.begin(), stop_names.end());

        std::vector<geo::Coordinates> geo_coords;
        for (transport_catalogue::Stop* stop : stops_in_routes) {
            geo_coords.push_back(stop->coords);
        }
        const SphereProjector proj{
            geo_coords.begin(), geo_coords.end(), settings.width, settings.height, settings.padding
        };
        svg::Document doc;
        AddLines(buses, settings, doc, bus_names, proj);
        AddLineTexts(buses, settings, doc, bus_names, proj);

        AddStops(stops, settings, doc, stop_names, proj);
        AddStopNames(stops, settings, doc, stop_names, proj);

        doc.Render(ostream);
    }

    void AddLines(
        std::map<std::string_view, transport_catalogue::Bus*>* buses,
        MapSettings& settings,
        svg::Document& doc,
        std::vector<std::string_view>& bus_names,
        const SphereProjector& proj
    ) {
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
                poly.AddPoint(proj(stop->coords));
            }
            doc.Add(poly);
        }
    }

    void AddLineTexts(
        std::map<std::string_view, transport_catalogue::Bus*>* buses,
        MapSettings& settings,
        svg::Document& doc,
        std::vector<std::string_view>& bus_names,
        const SphereProjector& proj
    ) {
        for (size_t i=0; i<bus_names.size();++i) {
            int color_num = i % settings.color_palette.size();
            svg::Color bus_color = settings.color_palette[color_num];
            transport_catalogue::Bus* bus = buses->at(bus_names[i]);

            AddTextWithBackground(
                settings,
                doc,
                proj,
                bus->first->coords,
                settings.bus_label_offset,
                bus->name,
                bus_color,
                settings.bus_label_font_size,
                true
            );
            if (!bus->is_roundtrip) {
                AddTextWithBackground(
                    settings,
                    doc,
                    proj,
                    bus->last->coords,
                    settings.bus_label_offset,
                    bus->name,
                    bus_color,
                    settings.bus_label_font_size,
                    true
                );
            }
        }
    }

    void AddTextWithBackground(
        MapSettings& settings,
        svg::Document& doc,
        const SphereProjector& proj,
        geo::Coordinates& coords,
        std::pair<double, double> offset,
        std::string& data,
        svg::Color& text_color,
        int font_size,
        bool is_bold
    ) {
        svg::Text text;
        text.SetPosition(proj(coords))
            .SetOffset(offset)
            .SetFontSize(font_size)
            .SetFontFamily("Verdana")
            .SetData(data);
        if (is_bold) {
            text.SetFontWeight("bold");
        }
        svg::Text underlayer{text};
        underlayer.SetFillColor(settings.underlayer_color)
                  .SetStrokeColor(settings.underlayer_color)
                  .SetStrokeWidth(settings.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor(text_color);

        doc.Add(underlayer);
        doc.Add(text);
    }

    void AddStops(
        std::map<std::string_view, transport_catalogue::Stop*>* stops,
        MapSettings& settings,
        svg::Document& doc,
        std::vector<std::string_view>& stop_names,
        const SphereProjector& proj
    ) {
        for (auto stop_name : stop_names) {
            transport_catalogue::Stop* stop = stops->at(stop_name);
            svg::Circle cir;
            cir.SetCenter(proj(stop->coords))
               .SetRadius(settings.stop_radius)
               .SetFillColor("white");
            doc.Add(cir);
        }
    }

    void AddStopNames(
        std::map<std::string_view, transport_catalogue::Stop*>* stops,
        MapSettings& settings,
        svg::Document& doc,
        std::vector<std::string_view>& stop_names,
        const SphereProjector& proj
    ) {
        for (auto stop_name : stop_names) {
            transport_catalogue::Stop* stop = stops->at(stop_name);
            svg::Color black {"black"};
            AddTextWithBackground(
                settings,
                doc,
                proj,
                stop->coords,
                settings.stop_label_offset,
                stop->name,
                black,
                settings.stop_label_font_size,
                false
            );
        }
    }

}
