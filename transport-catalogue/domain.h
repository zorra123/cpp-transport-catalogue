#pragma once
#include <string>
#include <set>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <map>
#include <vector>
#include "geo.h"

//extern class TransportCatalogue;

namespace stops {
	class Stops {
		//friend class TransportCatalogue;
	public:
		struct Stop {
			std::string name;
			geo::Coordinates coords;
			std::set<std::string_view> buses;
			bool operator < (const Stop& other) const {
				return name < other.name;
			}
			bool operator == (const Stop& other) const {
				return name == other.name;
			}
		};
		void AddStop(Stop stop);
		Stop* GetStop(std::string_view name_stop) const;
		std::map<std::string_view, Stop*> GetAllStops() {
			return { container_stops_.begin(),container_stops_.end() };
		}
		auto StopsCBegin() const {
			return stops_.cbegin();
		}
		auto StopsCEnd() const {
			return stops_.cend();
		}
	private:
		int tmp() {
			return 10;
		}
		std::deque< Stop> stops_;
		std::unordered_map<std::string_view, Stop*> container_stops_;
	};
}

namespace buses {
	using namespace stops;
	class Buses {
		//friend extern class DataBase::TransportCatalogue;
	public:
		Buses(stops::Stops& stops) :stops_(stops) {};
		struct Bus {
			std::string name;
			std::vector< Stops::Stop*> stops_at_route;
			bool circle;
		};
		void AddBus(const std::string& num, const std::vector<std::string>& name_stops, bool flag_is_cirle_route);
		const Bus* GetBus(std::string_view num_bus) const;
		std::map<std::string_view, Bus*> GetAllBuses() const {
			return { container_buses_.begin(),container_buses_.end() };
		}
		auto BusesCBegin()const {
			return buses_.cbegin();
		}
		auto BusesCEnd()const {
			return buses_.cend();
		}
		const Bus* GetPtrFromContainer(std::string_view bus) {
			return container_buses_.at(bus);
		}
	private:
		std::deque< Bus> buses_;
		std::unordered_map<std::string_view,Bus*> container_buses_;
		stops::Stops& stops_;

	};
}
