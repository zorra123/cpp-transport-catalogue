#include "map_renderer.h"
#include <algorithm>

using namespace map_reader;
using namespace DataBase;

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    SphereProjector() = default;
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


enum SET_COLOR {
    ONLY_STROKE,
    ONLY_FILL,
    BOTH
};
template <typename T>
struct Visitor {
    Visitor(T& obj_, SET_COLOR set_color_) :obj(obj_), set_color(set_color_) {};
        T &obj;
        SET_COLOR set_color;
     void operator()( std::string str) const {
         if (set_color == SET_COLOR::ONLY_STROKE) {
             obj.SetStrokeColor(str);
         }
         else if (set_color == SET_COLOR::ONLY_FILL) {
             obj.SetFillColor(str);
         }
         else {
             obj.SetStrokeColor(str);
             obj.SetFillColor(str);
         }
    }
     void operator()( svg::Rgb color) const {
         if (set_color == SET_COLOR::ONLY_STROKE) {
             obj.SetStrokeColor(color);
         }
         else if (set_color == SET_COLOR::ONLY_FILL) {
             obj.SetFillColor(color);
         }
         else {
             obj.SetStrokeColor(color);
             obj.SetFillColor(color);
         }
    }
     void operator()( svg::Rgba color) const {
         if (set_color == SET_COLOR::ONLY_STROKE) {
             obj.SetStrokeColor(color);
         }
         else if (set_color == SET_COLOR::ONLY_FILL) {
             obj.SetFillColor(color);
         }
         else {
             obj.SetStrokeColor(color);
             obj.SetFillColor(color);
         }
    }
};

class MapRendering {
public:
    MapRendering(Settings& settings, TransportCatalogue& catalog) :settings_(settings), catalog_(catalog) {};
    void AddPolyline();
    void AddTextMarshrutov();
    void AddCircle();
    void AddTextForStops();
    void Rendering(std::ostream& out);
private:
    //этот метод нужен для создания упорядоченной мапы из названия автобусов и координат их маршрутов.
    std::map<std::string, std::vector<geo::Coordinates>> CoordsForBuses();
    SphereProjector CreateProj(std::map<std::string, std::vector<geo::Coordinates>> coords_for_buses, double WIDTH, double HEIGHT, double PADDING);


private:
    Settings& settings_;
    TransportCatalogue& catalog_;

    SphereProjector proj;
    svg::Document doc;

};

std::stringstream map_reader::Rendering(Settings& settings, DataBase::TransportCatalogue& catalog)
{
    std::stringstream str;
    MapRendering map_rend(settings, catalog);
    map_rend.AddPolyline();
    map_rend.AddTextMarshrutov();
    map_rend.AddCircle();
    map_rend.AddTextForStops();
    map_rend.Rendering(str);
    return str;
}

void MapRendering::AddPolyline()
{
    //получаем мапу из автобусов и их остановок
    auto coords_for_buses = CoordsForBuses();
    //создаем проектор
    proj = CreateProj(coords_for_buses, settings_.width, settings_.height, settings_.padding);
    int it = 1;
    for (const auto& [bus_name, bus] : coords_for_buses) {
        svg::Polyline polyline = svg::Polyline()
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetFillColor("none");
        for (const auto& geo_coord : bus) {
            const svg::Point screen_coord = proj(geo_coord);
            polyline.AddPoint(screen_coord);
        }

        int size_of_color_palette = settings_.color_palette.size();
        int num_color_from_color_palette = it % size_of_color_palette;
        if (num_color_from_color_palette) {
            std::visit(Visitor{ polyline ,SET_COLOR::ONLY_STROKE }, settings_.color_palette.at(num_color_from_color_palette - 1));
        }
        else {
            std::visit(Visitor{ polyline ,SET_COLOR::ONLY_STROKE }, settings_.color_palette.at(size_of_color_palette - 1));
        }

        ++it;
        doc.Add(polyline);
    }

}

void MapRendering::AddTextMarshrutov() {
    struct Name {
        svg::Text podloshka;
        svg::Text name;
    };
    std::map<std::string, std::vector<Name>> buses;
    auto vec_buses = catalog_.GetAllBuses();
    int it = 1;
    for (const auto& [name_bus, bus] : vec_buses) {
        if (!bus->stops_at_route.empty()) {
            Name bus_svg_coords;

            svg::Point point = proj({ bus->stops_at_route.at(0)->coords.lat, bus->stops_at_route.at(0)->coords.lng });
            svg::Text same_settings = svg::Text()
                .SetData(std::string{ name_bus })
                .SetPosition(point)
                .SetOffset(svg::Point{ settings_.bus_label_offset.at(0),settings_.bus_label_offset.at(1) })
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold");
            bus_svg_coords.name = same_settings;
            bus_svg_coords.podloshka = same_settings;
            std::visit(Visitor{ bus_svg_coords.podloshka ,SET_COLOR::BOTH }, settings_.underlayer_color);

            bus_svg_coords.podloshka.SetStrokeWidth(settings_.underlayer_width);
            bus_svg_coords.podloshka.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            bus_svg_coords.podloshka.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            //определяем какой цвет нужен через остаток от деления, в визиторе все сделается
            int size_of_color_palette = settings_.color_palette.size();
            int num_color_from_color_palette = it % size_of_color_palette;
            if (num_color_from_color_palette) {
                std::visit(Visitor{ bus_svg_coords.name ,SET_COLOR::ONLY_FILL }, settings_.color_palette.at(num_color_from_color_palette - 1));
            }
            else {
                std::visit(Visitor{ bus_svg_coords.name ,SET_COLOR::ONLY_FILL }, settings_.color_palette.at(size_of_color_palette - 1));
            }
            buses[bus->name].push_back(bus_svg_coords);

            //если не круговое движение и первая координата не равна послоедней, то добавляем конечную остановку, все параметры те же, только меням координату
            if (!bus->circle && bus->stops_at_route[0] != bus->stops_at_route.back()) {
                svg::Point point = proj({ bus->stops_at_route.back()->coords.lat, bus->stops_at_route.back()->coords.lng });
                bus_svg_coords.podloshka.SetPosition(point);
                bus_svg_coords.name.SetPosition(point);
                buses[bus->name].push_back(bus_svg_coords);
            }
        };
        ++it;
    }
    //заполнение документа
    for (auto& [name_bus, vec_Name] : buses) {
        for (auto& el : vec_Name) {
            doc.Add(el.podloshka);
            doc.Add(el.name);
        }
    }
}

void MapRendering::AddCircle()
{
    auto stops = catalog_.GetAllStops();
    for (auto& [name, value] : stops) {
        if (!value->buses.empty()) {
            svg::Circle circle;
            svg::Point point = proj({ value->coords.lat, value->coords.lng });
            circle.SetCenter(point);
            circle.SetRadius(settings_.stop_radius);
            circle.SetFillColor("white");
            doc.Add(circle);
        }
    }
}

void MapRendering::AddTextForStops()
{
    struct Name {
        svg::Text podloshka;
        svg::Text name;
    };
    auto stops = catalog_.GetAllStops();
    for (auto& [name, value] : stops) {
        if (!value->buses.empty()) {
            Name stop;
            svg::Point point = proj({ value->coords.lat, value->coords.lng });
            svg::Text same_settings = svg::Text()
                .SetData(std::string{ name })
                .SetPosition(point)
                .SetOffset({settings_.stop_label_offset.at(0),settings_.stop_label_offset.at(1)})
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana");
            stop.name = same_settings;
            stop.podloshka = same_settings;

            std::visit(Visitor{ stop.podloshka ,SET_COLOR::BOTH }, settings_.underlayer_color);

            stop.podloshka.SetStrokeWidth(settings_.underlayer_width);
            stop.podloshka.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            stop.podloshka.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            stop.name.SetFillColor("black");

            doc.Add(stop.podloshka);
            doc.Add(stop.name);
        }
    }

}

void MapRendering::Rendering(std::ostream& out)
{
    doc.Render(out);

}

std::map<std::string, std::vector<geo::Coordinates>> MapRendering::CoordsForBuses()
{
    std::map<std::string, std::vector<geo::Coordinates>> res;
    auto vec_buses = catalog_.GetAllBuses();
    for (const auto& [key, bus] : vec_buses) {
        for (const auto& stop : bus->stops_at_route) {
            res[bus->name].push_back(geo::Coordinates{ stop->coords.lat,  stop->coords.lng });
        }
        //если круговое движение, вставляем остановки в обратном порядкке, исключая последнюю
        if (!bus->stops_at_route.empty() && !bus->circle) {
            res[bus->name].insert(res[bus->name].end(), std::next(res[bus->name].rbegin()), res[bus->name].rend());
        }
    }
    return res;
}

SphereProjector MapRendering::CreateProj(std::map<std::string, std::vector<geo::Coordinates>> coords_for_buses, double WIDTH, double HEIGHT, double PADDING)
{
    std::vector<geo::Coordinates> all_coords;
    //проходим по всем векторам и копируем их в результирующий, таким образом у нас в одном векторе будут все координаты
    for (auto& [key, value] : coords_for_buses) {
        if (!value.empty()) {
            all_coords.insert(all_coords.end(), value.begin(), value.end());
        }
    }
    return SphereProjector{
        all_coords.begin(), all_coords.end(), WIDTH, HEIGHT, PADDING
    };
}

