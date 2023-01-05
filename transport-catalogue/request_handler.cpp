#include "request_handler.h"
using namespace Request;
using namespace DataBase;
using namespace std::string_literals;

void RequestHandler::AddStop(std::string name, double latitude, double longitude)
{
	catalog_.AddStop(name, latitude, longitude);

}

void RequestHandler::AddDistanceToAnotherStops(std::string name_bus_from, std::string name_bus_to, int dist)
{
	distance_to_another_stops[name_bus_from].push_back({ name_bus_to, dist });
}

void Request::RequestHandler::LoadDistanceToCatalog()
{
	catalog_.InputAllDistance(distance_to_another_stops);
}

void RequestHandler::AddBus(std::string name, std::vector<std::string> name_stops, bool flag_is_cirle_route)
{
	catalog_.AddBus(name, name_stops, flag_is_cirle_route);
}

void RequestHandler::AddMapSettings(map_reader::Settings& settings)
{
	settings_ = settings;
}

void RequestHandler::RequestBus(std::string bus_name, int id_req)
{
	//json::Dict map_for_bus;
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

		//map_for_bus["curvature"] = (calculate_answer.distance_real / calculate_answer.distance_geographical);
		//map_for_bus["route_length"] = calculate_answer.distance_real;
		//map_for_bus["stop_count"] = num_stops;
		//map_for_bus["unique_stop_count"] = calculate_answer.num_unic_stops;
		//map_for_bus["request_id"] = id_req;
	}
	else {
		map_for_bus_.Key("error_message"s).Value("not found"s);
		map_for_bus_.Key("request_id"s).Value(id_req);

		//map_for_bus["error_message"] = "not found"s;
		//map_for_bus["request_id"] = id_req;
	}
	//answer.push_back(map_for_bus);
	//answer.push_back(map_for_bus_.EndDict().Build());
	build.Value(map_for_bus_.EndDict().Build());
}

void RequestHandler::RequestStop(std::string stop_name, int id_req)
{
	//json::Dict map_for_stop;

	json::Builder map_for_stop_;
	map_for_stop_.StartDict();

	auto stop_ptr = catalog_.GetStop(stop_name);
	if (stop_ptr) {
		if (stop_ptr->buses.empty()) {
			//map_for_stop["buses"] = json::Array{};
			//map_for_stop["request_id"] = id_req;

			map_for_stop_.Key("buses"s).Value(json::Array{});
			map_for_stop_.Key("request_id"s).Value(id_req);
		}
		else {
			//json::Array buses_for_this_stop;

			json::Builder buses_for_this_stop_;
			buses_for_this_stop_.StartArray();

			for (const auto& el : stop_ptr->buses) {
				//buses_for_this_stop.push_back({ std::string {el} });
				buses_for_this_stop_.Value(std::string{ el });
			}

			//map_for_stop["buses"] = buses_for_this_stop;
			//map_for_stop["request_id"] = id_req;
			
			map_for_stop_.Key("buses"s).Value(buses_for_this_stop_.EndArray().Build());
			map_for_stop_.Key("request_id"s).Value(id_req);


		}
	}
	else {
		//map_for_stop["error_message"] = "not found";
		//map_for_stop["request_id"] = id_req;

		map_for_stop_.Key("error_message"s).Value("not found"s);
		map_for_stop_.Key("request_id").Value(id_req);
	}

	//answer.push_back(map_for_stop);
	//answer.push_back(map_for_stop_.EndDict().Build());
	build.Value(map_for_stop_.EndDict().Build());
}

void RequestHandler::RequestSvg(int id_req)
{
	//json::Dict map_for_svg;
	//std::stringstream str_of_svg = std::move(map_reader::Render(settings_, catalog_));
	//map_for_svg["map"] = str_of_svg.str();
	//map_for_svg["request_id"] = id_req;
	//answer.push_back(map_for_svg);

	json::Builder map_for_svg_;
	map_for_svg_.StartDict();
	std::stringstream str_of_svg = std::move(map_reader::Render(settings_, catalog_));
	map_for_svg_.Key("map"s).Value(str_of_svg.str());
	map_for_svg_.Key("request_id"s).Value(id_req);
	//answer.push_back(map_for_svg_.EndDict().Build());
	build.Value(map_for_svg_.EndDict().Build());
}

void RequestHandler::PrintAnswer(std::ostream& out)
{
	json::Print(json::Document(build.EndArray().Build()), out);
}
