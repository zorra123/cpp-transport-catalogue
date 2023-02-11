#include "svg.h"
#include <map>
namespace svg {

    using namespace std::literals;
    std::ostream& operator<<(std::ostream& out, const svg::Color& color) {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const svg::StrokeLineCap& obj) {
        switch (obj)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt";
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round";
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square";
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const svg::StrokeLineJoin& obj) {
        switch (obj)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter";
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round";
            break;
        default:
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (const auto& el : points_) {
            out << el.x << "," << el.y;
            if (&el != &points_.back()) {
                out << " ";
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/> "sv;
    }

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        for (auto& el : data) {

            switch (el)
            {
            case '\"':
                data_ += "&quot;";
                break;
            case '\'':
                data_ += "&apos;";
                break;
            case '<':
                data_ += "&lt;";
                break;
            case '>':
                data_ += "&gt;";
                break;
            case '&':
                data_ += "&amp;";
                break;
            default:
                data_ += el;
                break;
            }
        }
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " ";
        out << "x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" ";
        out << "dx=\"" << offset_.x << "\" " << "dy=\"" << offset_.y << "\" ";
        out << "font-size=\"" << size_ << "\" ";
        if (!font_family_.empty()) {
            out << "font-family=\"" << font_family_ << "\" ";
        }
        if (!font_weight_.empty()) {
            out << "font-weight=\"" << font_weight_ << "\" ";
        }
        out << ">" << data_ << "</text>";
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& el : objects_) {

            RenderContext ctx(out, 2, 2);
            el.get()->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg
