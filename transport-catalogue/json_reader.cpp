#include <iostream>
#include <unordered_map>
#include <string>
#include <string_view>
#include <list>
#include <algorithm>
#include <memory>
#include "json.h"
#include "json_reader.h"
#include <fstream>
#include "map_renderer.h"


using namespace std::string_literals;
using namespace DataBase;

//ReadInputData(функция) вызывается от переданного потока ввода и класса базы, там создается класс, закрытый извне. В этом классе, 
//сначала в конструкторе происходит заполнение Stpos,а для Bus создается временный list структр Bus.
//В деструторе будем класть Bus в базу данных.
//

class ReadingInputData {
private:
	struct Bus {
		std::string num;
		std::vector<std::string> name_stops;
		bool flag_is_cirle_route;
	};
public:
	ReadingInputData(std::istream& stream, TransportCatalogue& catalog) :catalog_(catalog), stream_(stream) {	}

	//тут читаются все остановки и заполняется база по ним
	void InsertStops() {
		std::string str;
		std::string line;
		while (getline(stream_, line))
		{
			str += line;
		}

		std::vector<std::string_view> res;

		auto ptr_map_requsts = std::make_unique<json::Document>(json::LoadJSON(str));
		auto& map_requsts = ptr_map_requsts->GetRoot().AsMap();
		auto& base_requests = map_requsts.at("base_requests"s);

		json::Array bus_arr;

		std::for_each(base_requests.AsArray().begin(), base_requests.AsArray().end(), [&](auto& el) {
			if (el.AsMap().at("type").AsString() == "Stop") {
				TransportCatalogue::Stop stop;
				stop.name = el.AsMap().at("name").AsString();
				stop.coords = { el.AsMap().at("latitude").AsDouble(), el.AsMap().at("longitude").AsDouble() };
				catalog_.AddStop(stop);
				for (auto& [key, value] : el.AsMap().at("road_distances").AsMap()) {
					distance_to_another_stops[stop.name].push_back({ key,value.AsInt() });
				}
			}
			else {
				bus_arr.push_back(el);
			}
			});
		catalog_.InputAllDistance(distance_to_another_stops);
		buses_ = std::make_unique<json::Array>(bus_arr);
		stat_requests_ = std::make_unique<json::Array>(map_requsts.at("stat_requests"s).AsArray());
		if (map_requsts.count("render_settings")) {
			render_settings_ = std::make_unique<json::Dict>(map_requsts.at("render_settings"s).AsMap());
		}
	}
	//чтение автобусов и заполнение базы
	void InsertBuses() {
		for (auto& el : *(buses_.get())) {
			Bus bus;
			bus.num = el.AsMap().at("name").AsString();
			for (auto& bus_stops : el.AsMap().at("stops").AsArray()) {
				bus.name_stops.push_back(bus_stops.AsString());
			}
			bus.flag_is_cirle_route = el.AsMap().at("is_roundtrip").AsBool();
			catalog_.AddBus(bus.num, bus.name_stops, bus.flag_is_cirle_route);
		}
	}

	void RenderSvg() {
		if (render_settings_) {
			map_reader::Settings settings;

			settings.width = render_settings_->at("width").AsDouble();
			settings.height = render_settings_->at("height").AsDouble();
			settings.padding = render_settings_->at("padding").AsDouble();

			settings.stop_radius = render_settings_->at("stop_radius").AsDouble();
			settings.line_width = render_settings_->at("line_width").AsDouble();
			settings.bus_label_font_size = render_settings_->at("bus_label_font_size").AsDouble();

			settings.bus_label_offset = {render_settings_->at("bus_label_offset").AsArray().at(0).AsDouble(),
										render_settings_->at("bus_label_offset").AsArray().at(1).AsDouble()};

			settings.stop_label_font_size = render_settings_->at("stop_label_font_size").AsDouble();

			settings.stop_label_offset = { render_settings_->at("stop_label_offset").AsArray().at(0).AsDouble(),
										render_settings_->at("stop_label_offset").AsArray().at(1).AsDouble() };

			if (render_settings_->at("underlayer_color").IsString()) {
				settings.underlayer_color = render_settings_->at("underlayer_color").AsString();
			}
			else {
				if (render_settings_->at("underlayer_color").AsArray().size() == 3) {
					settings.SetColor(svg::Rgb(render_settings_->at("underlayer_color").AsArray().at(0).AsInt(),
						render_settings_->at("underlayer_color").AsArray().at(1).AsInt(),
						render_settings_->at("underlayer_color").AsArray().at(2).AsInt()),map_reader::underlayer_color);
				}
				else if (render_settings_->at("underlayer_color").AsArray().size() == 4) {
					settings.SetColor(svg::Rgba(render_settings_->at("underlayer_color").AsArray().at(0).AsInt(),
						render_settings_->at("underlayer_color").AsArray().at(1).AsInt(),
						render_settings_->at("underlayer_color").AsArray().at(2).AsInt(),
						render_settings_->at("underlayer_color").AsArray().at(3).AsDouble()),map_reader::underlayer_color);
				}
			}
			settings.underlayer_width = render_settings_->at("underlayer_width").AsDouble();

			for (auto& el : render_settings_->at("color_palette").AsArray()) {
				if (el.IsString()) {
					settings.SetColor(el.AsString(), map_reader::color_palette);
				}
				else {
					if (el.AsArray().size() == 3) {
						settings.SetColor(svg::Rgb(el.AsArray().at(0).AsInt(),
							el.AsArray().at(1).AsInt(),
							el.AsArray().at(2).AsInt()), map_reader::color_palette);
					}
					else if (el.AsArray().size() == 4) {
						settings.SetColor(
							svg::Rgba(el.AsArray().at(0).AsInt(),
								el.AsArray().at(1).AsInt(),
								el.AsArray().at(2).AsInt(),
								el.AsArray().at(3).AsDouble()),map_reader::color_palette);
					}
				}
			}

			str_of_svg =  std::move(map_reader::Rendering(settings, catalog_));

		}
	}
	//печать ответов на req
	void PrintReq() {
		json::Array answer;
		json::Dict map_for_bus;
		json::Dict map_for_stop;
		json::Dict map_for_svg;

		for (auto& el : *(stat_requests_.get())) {
			if (el.AsMap().at("type").AsString() == "Bus") {
				auto bus_ptr = catalog_.GetBus(el.AsMap().at("name").AsString());
				if (bus_ptr) {
					auto calculate_answer = catalog_.CalculateRealDistance(bus_ptr);
					int num_stops;
					bus_ptr->circle ? num_stops = bus_ptr->stops_at_route.size() : num_stops = bus_ptr->stops_at_route.size() * 2 - 1;
					map_for_bus["curvature"] = (calculate_answer.distance_real / calculate_answer.distance_geographical);
					map_for_bus["route_length"] = calculate_answer.distance_real;
					map_for_bus["stop_count"] = num_stops;
					map_for_bus["unique_stop_count"] = calculate_answer.num_unic_stops;
					map_for_bus["request_id"] = el.AsMap().at("id").AsInt();
				}
				else {
					map_for_bus["error_message"] = "not found"s;
					map_for_bus["request_id"] = el.AsMap().at("id").AsInt();
				}
				answer.push_back(map_for_bus);
				map_for_bus.clear();
			}
			else if (el.AsMap().at("type").AsString() == "Stop") {
				auto stop_ptr = catalog_.GetStop(el.AsMap().at("name").AsString());
				if (stop_ptr) {
					if (stop_ptr->buses.empty()) {
						map_for_stop["buses"] = json::Array{};
						map_for_stop["request_id"] = el.AsMap().at("id").AsInt();
					}
					else {
						json::Array buses_for_this_stop;
						for (const auto& el : stop_ptr->buses) {
							buses_for_this_stop.push_back({ std::string {el} });
						}
						map_for_stop["buses"] = buses_for_this_stop;
						map_for_stop["request_id"] = el.AsMap().at("id").AsInt();
					}
				}
				else {
					map_for_stop["error_message"] = "not found";
					map_for_stop["request_id"] = el.AsMap().at("id").AsInt();
				}
				answer.push_back(map_for_stop);
				map_for_stop.clear();
			}
			else {
				map_for_svg["map"] = str_of_svg.str();
				map_for_svg["request_id"] = el.AsMap().at("id").AsInt();
				answer.push_back(map_for_svg);
			}
		}
		//std::ofstream file("my_out.json");
		json::Print(answer, std::cout);

	}
private:
	TransportCatalogue& catalog_;
	std::list<Bus> bus_str_;
	std::unique_ptr<json::Array> buses_;
	std::unique_ptr<json::Array> stat_requests_;
	std::unique_ptr<json::Dict> render_settings_;
	std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> distance_to_another_stops;
	std::istream& stream_;
	std::stringstream str_of_svg;
};

void ReadInputData(std::istream& stream, TransportCatalogue& catalog)
{
	ReadingInputData mydata(stream, catalog);
	mydata.InsertStops();
	mydata.InsertBuses();
	mydata.RenderSvg();
	mydata.PrintReq();
}
