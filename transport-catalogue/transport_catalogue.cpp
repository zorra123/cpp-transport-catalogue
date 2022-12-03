#include "transport_catalogue.h"

using namespace DataBase;
using namespace stops;
using namespace buses;


void TransportCatalogue::AddBus(std::string& num, std::vector<std::string>& name_stops, bool flag_is_cirle_route) {
	Bus bus;
	buses_.push_back(bus);

	buses_.back().name = num;
	for (auto& el : name_stops) {
		buses_.back().stops_at_route.push_back(&*container_stops_.at(el));
		container_stops_[el]->buses.insert(buses_.back().name);
	}
	buses_.back().circle = flag_is_cirle_route;
	container_buses_[buses_.back().name] = &buses_.back();
}

TransportCatalogue::Distance TransportCatalogue::CalculateRealDistance(const Bus* bus)const {
	int distance_real = 0;
	double distance_geographical = 0;
	std::set< std::string_view> unic_stops;
	//хочу за один проход по вектору посчитать реальное и географическое расстоляние
	//нужно будет учесть, если маршрут из одной остановки???
	for (int first = 0, second = 1; second < bus->stops_at_route.size(); ++first, ++second) {
		auto ptr1 = bus->stops_at_route[first];
		auto ptr2 = bus->stops_at_route[second];
		//для географической дистанции
		distance_geographical += ComputeDistance(geo::Coordinates{ bus->stops_at_route[first]->coords.lat,	bus->stops_at_route[first]->coords.lng },
			geo::Coordinates{ bus->stops_at_route[second]->coords.lat,	bus->stops_at_route[second]->coords.lng });
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

	return Distance{ distance_real ,distance_geographical,(int)unic_stops.size() };
}
