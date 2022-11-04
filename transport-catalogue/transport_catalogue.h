#pragma once
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <string>
#include <vector>
#include <string_view>
#include "geo.h"

namespace DataBase {
	class TransportCatalogue {
	public:
		struct Stop {
			std::string name;
			std::pair<double, double> coords;
			std::set<std::string_view> buses;
		};
		struct Bus {
			std::string name;
			std::vector< Stop*> stops_at_route;
			bool circle;
		};
	private:
		struct DistanceBetweenStops {
			Stop* first_station = nullptr;
			Stop* second_station = nullptr;
			bool operator==(const DistanceBetweenStops& rhs) const {
				return first_station->name == rhs.first_station->name && second_station->name == rhs.second_station->name;
			}
			bool operator<(const DistanceBetweenStops& rhs) const {
				return first_station->name < rhs.first_station->name;
			}
		};
		struct DistanceBetweenStops_Hasher_ {
			size_t operator()(const DistanceBetweenStops& el)const {
				return (size_t)el.first_station + ((size_t)el.second_station * 37);
			}
		};
		struct Distance {
			int distance_real;
			double distance_geographical;
			int num_unic_stops;
		};

	public:
		TransportCatalogue() = default;
		void AddStop(Stop& stop);

		void AddBus(std::string& num, std::vector<std::string>& name_stops, bool flag_is_cirle_route);

		const Bus* GetBus(std::string_view num_bus) const;

		const Stop* GetStop(std::string_view name_stop) const;

		template <typename T>
		void InputAllDistance(T& distance_to_another_stops) {
			for (auto& [first_station, vec_second_stations] : distance_to_another_stops) {
				for (auto& second_station : vec_second_stations) {
					auto* ptr_first_station = container_stops_[first_station];
					auto* ptr_second_station = container_stops_[second_station.first];
					DistanceBetweenStops op;
					op.first_station = ptr_first_station;
					op.second_station = ptr_second_station;
					map_for_distance[op] = second_station.second;
				}
			}
		}

		Distance CalculateRealDistance(const Bus* bus)const;

	private:
		std::deque< Stop> stops_;
		std::unordered_map<std::string_view, Stop*> container_stops_;

		std::deque< Bus> buses_;
		std::unordered_map<std::string_view, Bus*> container_buses_;

		std::unordered_map< DistanceBetweenStops, int, DistanceBetweenStops_Hasher_> map_for_distance;
	};
}
