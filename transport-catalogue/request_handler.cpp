#include "request_handler.h"
using namespace Request;
using namespace DataBase;
using namespace std::string_literals;

void RequestHandler::AddStop(const std::string& name, double latitude, double longitude)
{
	catalog_.AddStop(name, latitude, longitude);
}

void RequestHandler::AddDistanceToAnotherStops(const std::string& name_bus_from,const std::string& name_bus_to, int dist)
{
	distance_to_another_stops[name_bus_from].push_back({ name_bus_to, dist });
}

void Request::RequestHandler::LoadDistanceToCatalog()
{
	catalog_.InputAllDistance(distance_to_another_stops);
}

void RequestHandler::AddBus(const std::string& name, const std::vector<std::string> &name_stops, bool flag_is_cirle_route)
{
	catalog_.AddBus(name, name_stops, flag_is_cirle_route);
}

void RequestHandler::AddMapSettings(const map_reader::Settings& settings)
{
	settings_ = settings;
}

void RequestHandler::RequestBus(const std::string& bus_name, int id_req)
{
	json::Builder map_for_bus_;
	map_for_bus_.StartDict();
	auto bus_ptr = catalog_.GetBus(bus_name);
	if (bus_ptr) {
		auto calculate_answer = catalog_.CalculateRealDistance(bus_ptr);
		int num_stops;
		bus_ptr->circle ? num_stops = bus_ptr->stops_at_route.size() : num_stops = bus_ptr->stops_at_route.size() * 2 - 1;

		map_for_bus_.Key("curvature"s).Value((calculate_answer.distance_real / calculate_answer.distance_geographical));
		map_for_bus_.Key("route_length"s).Value(calculate_answer.distance_real);
		map_for_bus_.Key("stop_count"s).Value(num_stops);
		map_for_bus_.Key("unique_stop_count"s).Value(calculate_answer.num_unic_stops);
		map_for_bus_.Key("request_id"s).Value(id_req);
	}
	else {
		map_for_bus_.Key("error_message"s).Value("not found"s);
		map_for_bus_.Key("request_id"s).Value(id_req);
	}
	build.Value(map_for_bus_.EndDict().Build());
}

void RequestHandler::RequestStop(const std::string& stop_name, int id_req)
{
	json::Builder map_for_stop_;
	map_for_stop_.StartDict();

	auto stop_ptr = catalog_.GetStop(stop_name);
	if (stop_ptr) {
		if (stop_ptr->buses.empty()) {
			map_for_stop_.Key("buses"s).Value(json::Array{});
			map_for_stop_.Key("request_id"s).Value(id_req);
		}
		else {
			json::Builder buses_for_this_stop_;
			buses_for_this_stop_.StartArray();

			for (const auto& el : stop_ptr->buses) {
				buses_for_this_stop_.Value(std::string{ el });
			}

			map_for_stop_.Key("buses"s).Value(buses_for_this_stop_.EndArray().Build());
			map_for_stop_.Key("request_id"s).Value(id_req);
		}
	}
	else {
		map_for_stop_.Key("error_message"s).Value("not found"s);
		map_for_stop_.Key("request_id").Value(id_req);
	}

	build.Value(map_for_stop_.EndDict().Build());
}

void RequestHandler::RequestSvg(int id_req)
{
	json::Builder map_for_svg_;
	map_for_svg_.StartDict();
	std::stringstream str_of_svg = std::move(map_reader::Render(settings_, catalog_));
	map_for_svg_.Key("map"s).Value(str_of_svg.str());
	map_for_svg_.Key("request_id"s).Value(id_req);
	build.Value(map_for_svg_.EndDict().Build());
}

void Request::RequestHandler::RequestRoute(const RouterTransport::TransportRouter::RoutingSettings& rout_settings, int id)
{
	if(transport_router_.Empty()){
		transport_router_.BuildGraph({ rout_settings.wait, rout_settings.velocity,rout_settings.from_station,rout_settings.to_station });
	}
	auto res = transport_router_.RequestRoute({ rout_settings.wait, rout_settings.velocity,rout_settings.from_station,rout_settings.to_station });
	json::Builder map_for_route_;
	if (res) {
		map_for_route_.StartDict();
		map_for_route_.Key("items").StartArray();
		double total_time = 0;
		for (auto& el : *res) {
			total_time += el.travel_time + el.wait_time;
			map_for_route_.StartDict().Key("stop_name").Value(std::string{ el.stop_from });
			map_for_route_.Key("time").Value(el.wait_time);
			map_for_route_.Key("type").Value("Wait"s).EndDict();

			map_for_route_.StartDict().Key("type").Value("Bus"s);
			map_for_route_.Key("bus").Value(std::string{ el.bus });
			map_for_route_.Key("span_count").Value(el.span_count);
			map_for_route_.Key("time").Value(el.travel_time).EndDict();
		}

		map_for_route_.EndArray();
		map_for_route_.Key("request_id").Value(id);
		map_for_route_.Key("total_time").Value(total_time).EndDict();
		build.Value(map_for_route_.Build());
	}
	else {
		map_for_route_.StartDict();
		map_for_route_.Key("request_id").Value(id);
		map_for_route_.Key("error_message").Value("not found"s).EndDict();
		build.Value(map_for_route_.Build());
		return;
	}
}

void RequestHandler::PrintAnswer(std::ostream& out)
{
	json::Print(json::Document(build.EndArray().Build()), out);
}
