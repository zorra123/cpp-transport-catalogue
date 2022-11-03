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

	public:
		TransportCatalogue() = default;
		void AddStop(Stop& stop);

		void AddBus(std::string num, std::vector<std::string> name_stops, bool flag_is_cirle_route);

		const Bus* BusGetter(std::string_view num_bus) const;

		const Stop* StopGetter(std::string_view name_stop) const;

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

		auto CalculateRealDistance(const Bus* bus)const {
			int distance_real = 0;
			double distance_geographical = 0;
			std::set< std::string_view> unic_stops;
			//хочу за один проход по вектору посчитать реальное и географическое расстоляние
			//нужно будет учесть, если маршрут из одной остановки???
			for (int first = 0, second = 1; second < bus->stops_at_route.size(); ++first, ++second) {
				auto ptr1 = bus->stops_at_route[first];
				auto ptr2 = bus->stops_at_route[second];
				//для географической дистанции
				distance_geographical += ComputeDistance(Coordinates{ bus->stops_at_route[first]->coords.first,	bus->stops_at_route[first]->coords.second },
					Coordinates{ bus->stops_at_route[second]->coords.first,	bus->stops_at_route[second]->coords.second });
				unic_stops.insert(bus->stops_at_route[first]->name);

				//для реальной дистанции, без учета того, что конечная остановка может быть кругом
				if (map_for_distance.count({ ptr1, ptr2 })) {
					distance_real += map_for_distance.at({ ptr1, ptr2 });
				}
				else if (map_for_distance.count({ ptr2, ptr1 })) {
					distance_real += map_for_distance.at({ ptr2, ptr1 });
				}
				//проверка на круговое движение
				if (!bus->circle) {
					if (map_for_distance.count({ ptr2, ptr1 })) {
						distance_real += map_for_distance.at({ ptr2, ptr1 });
					}
					else {
						distance_real += map_for_distance.at({ ptr1, ptr2 });
					}
				}
			}
			unic_stops.insert(bus->stops_at_route.back()->name);
			//проверка последняя остановка - круг, если да, то плюсуем в реальную дистанцию
			//P.S в map_for_distance круг будет храниться в виде "name","name", где name одинаковые.
			auto ptr = bus->stops_at_route.back();
			if (map_for_distance.count({ ptr, ptr })) {
				distance_real += map_for_distance.at({ ptr, ptr });
			}
			//для георафической дистанции, если движение круговое, то дистанциб умножить на два
			if (!bus->circle) {
				distance_geographical *= 2;
			}
			struct result {
				int distance_real;
				double distance_geographical;
				int num_unic_stops;
			} res{ distance_real, distance_geographical,(int)unic_stops.size() };

			return res;
		}

	private:
		std::deque< Stop> stops_;
		std::unordered_map<std::string_view, Stop*> container_stops_;

		std::deque< Bus> buses_;
		std::unordered_map<std::string_view, Bus*> container_buses_;

		std::unordered_map< DistanceBetweenStops, int, DistanceBetweenStops_Hasher_> map_for_distance;
	};
}
