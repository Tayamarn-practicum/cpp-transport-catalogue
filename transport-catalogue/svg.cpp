#include "svg.h"

#include <sstream>

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const Rgb rgb) {
        out << "rgb(" << static_cast<int>(rgb.red) << ","
                      << static_cast<int>(rgb.green) << ","
                      << static_cast<int>(rgb.blue) << ")";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Rgba rgba){
        out << "rgba(" << static_cast<int>(rgba.red) << ","
                       << static_cast<int>(rgba.green) << ","
                       << static_cast<int>(rgba.blue) << ","
                       << rgba.opacity << ")";
        return out;
    }

    void PrintColor(std::ostream& out, std::monostate) {
        out << "none"s;
    }

    void PrintColor(std::ostream& out, std::string color) {
        out << color;
    }

    void PrintColor(std::ostream& out, Rgb color) {
        out << color;
    }

    void PrintColor(std::ostream& out, Rgba color) {
        out << color;
    }

    std::ostream& operator<<(std::ostream& out, const Color color) {
        std::visit([&out](auto clr){PrintColor(out, clr);}, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap slc) {
        std::string lc;
        switch (slc)
        {
            case StrokeLineCap::BUTT:
                lc = "butt";
                break;
            case StrokeLineCap::ROUND:
                lc = "round";
                break;
            case StrokeLineCap::SQUARE:
                lc = "square";
                break;
        }

        out << lc;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin slj) {
        std::string lj;
        switch (slj)
        {
            case StrokeLineJoin::ARCS:
                lj = "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                lj = "bevel";
                break;
            case StrokeLineJoin::MITER:
                lj = "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                lj = "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                lj = "round";
                break;
        }

        out << lj;
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        bool first = true;
        for (auto point : points_) {
            if (first) {
                first = false;
            } else {
                out << " ";
            }
            out << point.x << "," << point.y;
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>";
    }

    // ---------- Text ----------------------

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string Text::EscapeData(const std::string& str) const {
        std::string res = str;
        ReplaceAll(res, "&", "&amp;");
        ReplaceAll(res, "<", "&lt;");
        ReplaceAll(res, ">", "&gt;");
        ReplaceAll(res, "'", "&apos;");
        ReplaceAll(res, "\"", "&quot;");
        return res;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" "
            << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" "
            << "font-size=\"" << size_ << "\"";
        if (font_weight_ != "") {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        if (font_family_ != "") {
            out << " font-family=\"" << font_family_ << "\"";
        }
        RenderAttrs(context.out);
        out << ">" << EscapeData(data_) << "</text>";
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        auto it = objects_.begin();
        while (it != objects_.end()) {
            (*it)->Render(ctx);
            ++it;
        }
        out << "</svg>"sv;
    }

}  // namespace svg
