#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include "geo.h"
#include "domain.h"

namespace DataBase {
	class TransportCatalogue{
	public:
		TransportCatalogue() :class_buses_(class_stops_) {};

		void AddBus(const std::string& num, const std::vector<std::string>& name_stops, bool flag_is_cirle_route);
		const buses::Buses::Bus* GetBus(std::string_view num_bus) const;
		auto GetAllBuses() const {
			return class_buses_.GetAllBuses();
		}

		void AddStop(const std::string& name, double latitude, double longitude);
		stops::Stops::Stop* GetStop(std::string_view name_stop) const;
		auto GetAllStops() {
			return class_stops_.GetAllStops();
		}

		template <typename T>
		void InputAllDistance(T& distance_to_another_stops) {
			for (auto& [first_station, vec_second_stations] : distance_to_another_stops) {
				for (auto& [name_of_second_station, distance] : vec_second_stations) {
					auto* ptr_first_station = class_stops_.GetStop(first_station);
					auto* ptr_second_station = class_stops_.GetStop(name_of_second_station);
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
		Distance CalculateRealDistance(const buses::Buses::Bus* bus)const;

		auto StopsCBegin()const {
			return class_stops_.StopsCBegin();
		}
		auto StopsCEnd()const {
			return class_stops_.StopsCEnd();
		}
		auto BusesCBegin()const {
			return class_buses_.BusesCBegin();
		}
		auto BusesCEnd()const {
			return class_buses_.BusesCEnd();
		}
		auto GetPtrToBus(std::string_view bus) {
			return class_buses_.GetPtrFromContainer(bus);
		}
		auto GetPtrToStop(std::string_view stop) {
			return class_stops_.GetStop(stop);
		}
		const auto GetDistance(const stops::Stops::Stop* first_station,
								const stops::Stops::Stop* second_station) const {
			if (map_for_distance.count({ first_station,second_station })) {
				return map_for_distance.at({ first_station,second_station });
			}
			else if (map_for_distance.count({ second_station,first_station })) {
				return map_for_distance.at({ second_station,first_station });
			}
			return 0;
			/*return map_for_distance.count({ first_station,second_station }) ? map_for_distance.at({ first_station,second_station }) : 
																				map_for_distance.at({ second_station,first_station });*/
		}
	private:
		struct DistanceBetweenStops {
			const stops::Stops::Stop* first_station = nullptr;
			const stops::Stops::Stop* second_station = nullptr;
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
		stops::Stops class_stops_;
		buses::Buses class_buses_;
	};
}
