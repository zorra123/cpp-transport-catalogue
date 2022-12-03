#pragma once
#include <string>
#include <set>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <map>
#include <vector>
#include "geo.h"

namespace stops {
	class Stops {
	public:
		struct Stop {
			std::string name;
			geo::Coordinates coords;
			std::set<std::string_view> buses;
		};
		void AddStop(Stop& stop);
		const Stop* GetStop(std::string_view name_stop) const;
		std::map<std::string_view, Stop*> GetAllStops() {
			return { container_stops_.begin(),container_stops_.end() };
		}
	protected:
		std::deque< Stop> stops_;
		std::unordered_map<std::string_view, Stop*> container_stops_;
	};
}

namespace buses {
	using namespace stops;
	class Buses {
	public:
		struct Bus {
			std::string name;
			std::vector< Stops::Stop*> stops_at_route;
			bool circle;
		};
		virtual void AddBus(std::string& num, std::vector<std::string>& name_stops, bool flag_is_cirle_route) = 0;
		const Bus* GetBus(std::string_view num_bus) const;
		std::map<std::string_view, Bus*> GetAllBuses() const {
			return { container_buses_.begin(),container_buses_.end() };
		}

	protected:
		std::deque< Bus> buses_;
		std::unordered_map<std::string_view, Bus*> container_buses_;

	};
}