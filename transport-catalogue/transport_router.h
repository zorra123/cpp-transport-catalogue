#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include "domain.h"
#include <iostream>
namespace RouterTransport {
	class TransportRouter {
	public: 
		struct Struct_for_graph {
			stops::Stops::Stop stop;
			bool transfer = false;
			bool operator < (const Struct_for_graph& other) const {
				return stop == other.stop ? transfer < other.transfer : stop < other.stop;
			}
		};
		
		struct Weight {
			double travel_time = 0;
			double wait_time = 0;
			std::string_view bus;
			std::string_view stop_from;
			std::string_view stop_to;
			int span_count = 0;
			bool operator > (const Weight& other) const {
				return travel_time + wait_time > other.travel_time + other.wait_time;
			}
			bool operator < (const Weight& other) const {
				return travel_time + wait_time < other.travel_time + other.wait_time;
			}
			Weight operator + (const Weight& other) const {
				return { travel_time + other.travel_time,wait_time + other.wait_time,{},{},{} };
			}
			bool Empty()const {
				return travel_time == 0 && wait_time == 0;
			}
		};

		struct RoutingSettings {
			//wait as min
			double wait = 0;
			//velocity as meters/min
			double velocity = 0;
			std::string from_station;
			std::string to_station;
		};
	public:
		TransportRouter(DataBase::TransportCatalogue& transport_cataloge) :transport_cataloge_(transport_cataloge) {};

		void BuildGraph(RoutingSettings settings);
		void BuildEdge();
		std::optional<std::vector<Weight>> RequestRoute(const RoutingSettings& rout_settings);
		bool Empty() const;
	private:
		bool is_empty_ = true;
		DataBase::TransportCatalogue& transport_cataloge_;
		RoutingSettings rout_settings_;
		graph::DirectedWeightedGraph <Weight> graph_;
		std::set<Struct_for_graph> set_for_graph_;
	};
}
