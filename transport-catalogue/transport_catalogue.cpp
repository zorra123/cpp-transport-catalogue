#include "transport_catalogue.h"

using namespace DataBase;

void TransportCatalogue::AddStop(Stop& stop) {
	stops_.push_back(stop);
	container_stops_[stops_.back().name] = &stops_.back();
}

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

const TransportCatalogue::Bus* TransportCatalogue::GetBus(std::string_view num_bus) const {
	if (container_buses_.count(num_bus)) {
		return container_buses_.at(num_bus);
	}
	else {
		return nullptr;
	}
}

const TransportCatalogue::Stop* TransportCatalogue::GetStop(std::string_view name_stop) const {
	if (container_stops_.count(name_stop)) {
		return container_stops_.at(name_stop);
	}
	else {
		return nullptr;
	}
}

TransportCatalogue::Distance TransportCatalogue::CalculateRealDistance(const Bus* bus)const {
	int distance_real = 0;
	double distance_geographical = 0;
	std::set< std::string_view> unic_stops;
	//���� �� ���� ������ �� ������� ��������� �������� � �������������� �����������
	//����� ����� ������, ���� ������� �� ����� ���������???
	for (int first = 0, second = 1; second < bus->stops_at_route.size(); ++first, ++second) {
		auto ptr1 = bus->stops_at_route[first];
		auto ptr2 = bus->stops_at_route[second];
		//��� �������������� ���������
		distance_geographical += ComputeDistance(Coordinates{ bus->stops_at_route[first]->coords.first,	bus->stops_at_route[first]->coords.second },
			Coordinates{ bus->stops_at_route[second]->coords.first,	bus->stops_at_route[second]->coords.second });
		unic_stops.insert(bus->stops_at_route[first]->name);

		//��� �������� ���������, ��� ����� ����, ��� �������� ��������� ����� ���� ������
		if (map_for_distance.count({ ptr1, ptr2 })) {
			distance_real += map_for_distance.at({ ptr1, ptr2 });
		}
		else if (map_for_distance.count({ ptr2, ptr1 })) {
			distance_real += map_for_distance.at({ ptr2, ptr1 });
		}
		//�������� �� �������� ��������
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
	//�������� ��������� ��������� - ����, ���� ��, �� ������� � �������� ���������
	//P.S � map_for_distance ���� ����� ��������� � ���� "name","name", ��� name ����������.
	auto ptr = bus->stops_at_route.back();
	if (map_for_distance.count({ ptr, ptr })) {
		distance_real += map_for_distance.at({ ptr, ptr });
	}
	//��� ������������� ���������, ���� �������� ��������, �� ��������� �������� �� ���
	if (!bus->circle) {
		distance_geographical *= 2;
	}

	return Distance{ distance_real ,distance_geographical,(int)unic_stops.size() };
}