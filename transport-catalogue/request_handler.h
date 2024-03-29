#pragma once
#include <string>
#include <unordered_map>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

#include"transport_router.h"
namespace Request {
	class RequestHandler {
	public:
		RequestHandler(DataBase::TransportCatalogue& catalog) :catalog_(catalog), transport_router(catalog_), transport_router_(catalog_){ build.StartArray(); };
		void AddStop(const std::string& name, double latitude, double longitude);
		void AddDistanceToAnotherStops(const std::string& name_bus_from, const std::string& name_bus_to, int dist);
		void LoadDistanceToCatalog();
		void AddBus(const std::string& name, const std::vector<std::string>& name_stops, bool flag_is_cirle_route);
		void AddMapSettings(const map_reader::Settings& settings);
		void RequestBus(const std::string& bus_name, int id_req);
		void RequestStop(const std::string& stop_name, int id_req);
		void RequestSvg(int id_req);
		void RequestRoute(const RouterTransport::TransportRouter::RoutingSettings& rout_settings, int id);
		void PrintAnswer(std::ostream& out);
	private:
		DataBase::TransportCatalogue& catalog_;
		RouterTransport::TransportRouter transport_router;
		std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> distance_to_another_stops;
		map_reader::Settings settings_;
		json::Array answer;
		json::Builder build;


		RouterTransport::TransportRouter transport_router_;
	};
}
