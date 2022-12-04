#include "domain.h"

using namespace stops;
using namespace buses;

void Stops::AddStop(Stop stop)
{
	stops_.push_back(stop);
	container_stops_[stops_.back().name] = &stops_.back();
}

Stops::Stop* stops::Stops::GetStop(std::string_view name_stop) const
{
	if (container_stops_.count(name_stop)) {
		return container_stops_.at(name_stop);
	}
	else {
		return nullptr;
	}
}

void buses::Buses::AddBus(std::string& num, std::vector<std::string>& name_stops, bool flag_is_cirle_route)
{
	Bus bus;
	buses_.push_back(bus);

	buses_.back().name = num;
	for (auto& el : name_stops) {
		buses_.back().stops_at_route.push_back(stops_.GetStop(el));
		stops_.GetStop(el)->buses.insert(buses_.back().name);
	}
	buses_.back().circle = flag_is_cirle_route;
	container_buses_[buses_.back().name] = &buses_.back();
}

const Buses::Bus* buses::Buses::GetBus(std::string_view num_bus) const
{
	if (container_buses_.count(num_bus)) {
		return container_buses_.at(num_bus);
	}
	else {
		return nullptr;
	}
}
