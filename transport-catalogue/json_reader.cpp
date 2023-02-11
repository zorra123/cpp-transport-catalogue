#include <iostream>
#include <unordered_map>
#include <string>
#include <string_view>
#include <list>
#include <algorithm>
#include <memory>
#include <fstream>
#include "map_renderer.h"
#include "json.h"
#include "json_reader.h"
#include "transport_router.h"


using namespace std::string_literals;
using namespace DataBase;

class JsonReader {
private:
	struct Bus {
		std::string num;
		std::vector<std::string> name_stops;
		bool flag_is_cirle_route;
	};
public:
	JsonReader(std::istream& stream,  Request::RequestHandler& req) : stream_(stream), request_handler_(req) {	}

	void LoadStops();
	void ReadBuses();
	void LoadSvgSettings();
	void LoadRequests();
private:
	std::unique_ptr<json::Array> buses_;
	std::unique_ptr<json::Array> stat_requests_;
	std::unique_ptr<json::Dict> render_settings_;
	RouterTransport::TransportRouter::RoutingSettings routing_settings_;
	std::istream& stream_;
	Request::RequestHandler& request_handler_;
};

void ReadJson(std::istream& stream, Request::RequestHandler &req)
{
	JsonReader mydata(stream, req);
	mydata.LoadStops();
	mydata.ReadBuses();
	mydata.LoadSvgSettings();
	mydata.LoadRequests();
}

void JsonReader::LoadStops()
{
	std::string str;
	std::string line;
	while (getline(stream_, line))
	{
		str += line;
	}
	//for (int i = 0; i < 254; ++i) {
	//	getline(stream_, line);
	//	str += line;
	//}

	auto ptr_map_requsts = std::make_unique<json::Document>(json::LoadJSON(str));
	auto& map_requsts = ptr_map_requsts->GetRoot().AsDict();
	auto& base_requests = map_requsts.at("base_requests"s);

	json::Array bus_arr;

	std::for_each(base_requests.AsArray().begin(), base_requests.AsArray().end(), [&](auto& el) {
		if (el.AsDict().at("type").AsString() == "Stop") {

			std::string name = el.AsDict().at("name").AsString();
			double latitude = el.AsDict().at("latitude").AsDouble();
			double longitude = el.AsDict().at("longitude").AsDouble();

			request_handler_.AddStop(name, latitude, longitude);

			for (auto& [key, value] : el.AsDict().at("road_distances").AsDict()) {
				request_handler_.AddDistanceToAnotherStops(name, key, value.AsInt());
			}
		}
		else {
			bus_arr.push_back(el);
		}
		});
	request_handler_.LoadDistanceToCatalog();
	buses_ = std::make_unique<json::Array>(bus_arr);
	stat_requests_ = std::make_unique<json::Array>(map_requsts.at("stat_requests"s).AsArray());
	if (map_requsts.count("render_settings")) {
		render_settings_ = std::make_unique<json::Dict>(map_requsts.at("render_settings"s).AsDict());
	}
	if (map_requsts.count("routing_settings")) {
		//время задается в мин
		routing_settings_.wait = map_requsts.at("routing_settings"s).AsDict().at("bus_wait_time").AsDouble();
		//скорось сразу переведем из км/ч в м/мин
		static const double METER_AT_KM = 1000.0;
		static const double MIN_AT_HOUR = 60.0;
		routing_settings_.velocity = (double)map_requsts.at("routing_settings"s).AsDict().at("bus_velocity").AsDouble()* METER_AT_KM/ MIN_AT_HOUR;
	}
}

void JsonReader::ReadBuses()
{	
	for (auto& el : *(buses_.get())) {
		std::string bus_name = el.AsDict().at("name").AsString();
		std::vector<std::string> name_stops;
		for (auto& bus_stops : el.AsDict().at("stops").AsArray()) {
			name_stops.push_back(bus_stops.AsString());
		}
		bool flag_is_cirle_route = el.AsDict().at("is_roundtrip").AsBool();
		request_handler_.AddBus(bus_name, name_stops, flag_is_cirle_route);
	}
}

void JsonReader::LoadSvgSettings()
{
	if (render_settings_) {
		map_reader::Settings settings;

		settings.width = render_settings_->at("width").AsDouble();
		settings.height = render_settings_->at("height").AsDouble();
		settings.padding = render_settings_->at("padding").AsDouble();

		settings.stop_radius = render_settings_->at("stop_radius").AsDouble();
		settings.line_width = render_settings_->at("line_width").AsDouble();
		settings.bus_label_font_size = render_settings_->at("bus_label_font_size").AsDouble();

		settings.bus_label_offset = { render_settings_->at("bus_label_offset").AsArray().at(0).AsDouble(),
									render_settings_->at("bus_label_offset").AsArray().at(1).AsDouble() };

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
					render_settings_->at("underlayer_color").AsArray().at(2).AsInt()), map_reader::underlayer_color);
			}
			else if (render_settings_->at("underlayer_color").AsArray().size() == 4) {
				settings.SetColor(svg::Rgba(render_settings_->at("underlayer_color").AsArray().at(0).AsInt(),
					render_settings_->at("underlayer_color").AsArray().at(1).AsInt(),
					render_settings_->at("underlayer_color").AsArray().at(2).AsInt(),
					render_settings_->at("underlayer_color").AsArray().at(3).AsDouble()), map_reader::underlayer_color);
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
							el.AsArray().at(3).AsDouble()), map_reader::color_palette);
				}
			}
		}
		request_handler_.AddMapSettings(settings);
	}
}

void JsonReader::LoadRequests()
{
	for (auto& el : *(stat_requests_.get())) {
		if (el.AsDict().at("type").AsString() == "Bus") {
			std::string name_bus = el.AsDict().at("name").AsString();
			int id = el.AsDict().at("id").AsInt();
			request_handler_.RequestBus(name_bus, id);
		}
		else if (el.AsDict().at("type").AsString() == "Stop") {
			std::string name_stop = el.AsDict().at("name").AsString();
			int id = el.AsDict().at("id").AsInt();
			request_handler_.RequestStop(name_stop, id);
		}
		else if (el.AsDict().at("type").AsString() == "Route") {
			routing_settings_.from_station = el.AsDict().at("from").AsString();
			routing_settings_.to_station = el.AsDict().at("to").AsString();
			int id = el.AsDict().at("id").AsInt();
			request_handler_.RequestRoute(routing_settings_,id);
		}
		else {
			int id = el.AsDict().at("id").AsInt();
			request_handler_.RequestSvg(id);
		}
	}
}
