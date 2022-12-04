#pragma once
#include <string>
#include <unordered_map>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
namespace Request {

	class RequestHandler {
	public:
		RequestHandler(DataBase::TransportCatalogue& catalog) :catalog_(catalog) {};
		void AddStop(std::string name, double latitude, double longitude);
		void AddDistanceToAnotherStops(std::string name_bus_from, std::string name_bus_to, int dist);
		void LoadDistanceToCatalog();
		void AddBus(std::string name, std::vector<std::string>name_stops, bool flag_is_cirle_route);
		void AddMapSettings(map_reader::Settings& settings);
		void RequestBus(std::string bus_name, int id_req);
		void RequestStop(std::string stop_name, int id_req);
		void RequestSvg(int id_req);
		void PrintAnswer(std::ostream& out);
	private:
		DataBase::TransportCatalogue& catalog_;
		std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> distance_to_another_stops;
		map_reader::Settings settings_;
		json::Array answer;
	};
}