#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "geo.h"
#include "domain.h"

namespace DataBase {
	class TransportCatalogue :public stops::Stops, public buses::Buses {
	public:
		TransportCatalogue() = default;

		void AddBus(std::string& num, std::vector<std::string>& name_stops, bool flag_is_cirle_route) override;

		template <typename T>
		void InputAllDistance(T& distance_to_another_stops) {
			for (auto& [first_station, vec_second_stations] : distance_to_another_stops) {
				for (auto& [name_of_second_station, distance] : vec_second_stations) {
					auto* ptr_first_station = container_stops_[first_station];
					auto* ptr_second_station = container_stops_[name_of_second_station];
					DistanceBetweenStops tmp_struct;
					tmp_struct.first_station = ptr_first_station;
					tmp_struct.second_station = ptr_second_station;
					map_for_distance[tmp_struct] = distance;
				}
			}
		}
		struct Distance {
			int distance_real;
			double distance_geographical;
			int num_unic_stops;
		};
		Distance CalculateRealDistance(const Bus* bus)const;
			
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

	private:
		std::unordered_map< DistanceBetweenStops, int, DistanceBetweenStops_Hasher_> map_for_distance;
	};
}
