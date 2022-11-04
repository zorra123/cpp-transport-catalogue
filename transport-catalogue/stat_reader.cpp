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
	StatReader_(std::istream& stream_input, std::ostream& stream_output_, TransportCatalogue& catalog) :catalog_(catalog), 
																stream_input_(stream_input), stream_output_(stream_output_) {
		std::string num;
		getline(stream_input_, num);
		for (int i = 0; i < std::stoi(num); ++i) {
			std::string str;
			getline(stream_input_, str);
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
		for (auto& [el, req] : list_requests) {
			if (req == REQUEST::BUS) {
				auto bus_ptr = catalog_.GetBus(el);
				if (bus_ptr) {
					auto calculate_answer = catalog_.CalculateRealDistance(bus_ptr);
					int num_stops;
					bus_ptr->circle ? num_stops = bus_ptr->stops_at_route.size() :num_stops = bus_ptr->stops_at_route.size() * 2 - 1;
					stream_output_ << "Bus "s << el << ": "s << num_stops << " stops on route, "s << calculate_answer.num_unic_stops << " unique stops, "
						<< calculate_answer.distance_real << " route length, "s 
						<< calculate_answer.distance_real / calculate_answer.distance_geographical << " curvature" << std::endl;
				}
				else {
					stream_output_ << "Bus "s << el << ": not found"s << std::endl;
				}
			}
			else if (req == REQUEST::STOP) {
				auto stop_ptr = catalog_.GetStop(el);
				if (stop_ptr) {
					if (stop_ptr->buses.empty()) {
						stream_output_ << "Stop "s << el << ": no buses" << std::endl;
					}
					else {
						stream_output_ << "Stop "s << el << ": buses";
						for (const auto& el : stop_ptr->buses) {
							stream_output_ << " "s << el;
						}
						stream_output_ << std::endl;
					}
				}
				else {
					stream_output_ << "Stop "s << el << ": not found";
				}
			}
		}
	}

private:
	TransportCatalogue& catalog_;

	std::istream& stream_input_;
	std::ostream& stream_output_;

	std::list<std::pair<std::string, REQUEST>> list_requests;
};


void StatReader(std::istream& stream_input, std::ostream& stream_output, DataBase::TransportCatalogue& catalog)
{
	StatReader_ reader(stream_input, stream_output, catalog);
}
