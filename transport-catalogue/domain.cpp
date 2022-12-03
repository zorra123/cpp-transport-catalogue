#include "domain.h"

using namespace stops;
using namespace buses;

void Stops::AddStop(Stop& stop)
{
	stops_.push_back(stop);
	container_stops_[stops_.back().name] = &stops_.back();
}

const Stops::Stop* stops::Stops::GetStop(std::string_view name_stop) const
{
	if (container_stops_.count(name_stop)) {
		return container_stops_.at(name_stop);
	}
	else {
		return nullptr;
	}
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
