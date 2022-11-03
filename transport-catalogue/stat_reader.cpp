#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <set>
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace std::string_literals;
using namespace DataBase;

class StatReader_ {
private:
	enum REQUEST {
		BUS,
		STOP
	};
public:
	StatReader_(std::istream& stream, TransportCatalogue& catalog) :catalog_(catalog), stream_(stream) {
		std::string num;
		getline(stream, num);
		for (int i = 0; i < std::stoi(num); ++i) {
			std::string str;
			getline(stream, str);
			auto start_request = str.find(' ');
			if (str.substr(0, start_request) == "Bus"s) {
				list_requests.push_back({ str.substr(start_request + 1, str.size() - start_request),REQUEST::BUS });
			}
			else if (str.substr(0, start_request) == "Stop"s) {
				list_requests.push_back({ str.substr(start_request + 1, str.size() - start_request),REQUEST::STOP });
			}
		}
	}
	~StatReader_() {
		/*std::fstream f;
		f.open("my.txt", std::ios::out);*/
		for (auto& [el, req] : list_requests) {
			if (req == REQUEST::BUS) {
				auto bus_ptr = catalog_.BusGetter(el);
				if (bus_ptr) {
					auto calculate_answer = catalog_.CalculateRealDistance(bus_ptr);
					int num_stops;
					bus_ptr->circle ? num_stops = bus_ptr->stops_at_route.size() :num_stops = bus_ptr->stops_at_route.size() * 2 - 1;
					std::cout << "Bus "s << el << ": "s << num_stops << " stops on route, "s << calculate_answer.num_unic_stops << " unique stops, "
						<< calculate_answer.distance_real << " route length, "s 
						<< calculate_answer.distance_real / calculate_answer.distance_geographical << " curvature" << std::endl;

				}
				else {
					std::cout << "Bus "s << el << ": not found"s << std::endl;
				}
			}
			else if (req == REQUEST::STOP) {
				auto stop_ptr = catalog_.StopGetter(el);
				if (stop_ptr) {
					if (stop_ptr->buses.empty()) {
						std::cout << "Stop "s << el << ": no buses" << std::endl;
					}
					else {
						std::cout << "Stop "s << el << ": buses";
						for (const auto& el : stop_ptr->buses) {
							std::cout << " "s << el;
						}
						std::cout << std::endl;
					}
				}
				else {
					std::cout << "Stop "s << el << ": not found" << std::endl;
				}
			}
		}
	}

private:
	TransportCatalogue& catalog_;
	std::istream& stream_;

	std::list<std::pair<std::string, REQUEST>> list_requests;

};

void StatReader(std::istream& stream, TransportCatalogue& catalog)
{
	StatReader_ reader(stream, catalog);
}
