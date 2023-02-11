#include "transport_router.h"


void RouterTransport::TransportRouter::BuildGraph(RoutingSettings settings)
{
	rout_settings_ = settings;
	for (auto it_by_buses = transport_cataloge_.BusesCBegin();
		it_by_buses != transport_cataloge_.BusesCEnd();
		++it_by_buses) {
		double is_distance_begin = 0, is_distance_end = 0;

		is_distance_begin = transport_cataloge_.GetDistance(*it_by_buses->stops_at_route.cbegin(),
			*it_by_buses->stops_at_route.cbegin());

		is_distance_end = transport_cataloge_.GetDistance(*(it_by_buses->stops_at_route.cend() - 1),
			*(it_by_buses->stops_at_route.cend() - 1));

		if (is_distance_begin) {
			set_for_graph_.insert({ **(it_by_buses->stops_at_route.begin()) ,true });
		}
		if (is_distance_end) {
			set_for_graph_.insert({ **(it_by_buses->stops_at_route.end()) ,true });
		}

		std::set<std::string_view> stops_at_route;
		for (auto it_by_stops_at_bus = it_by_buses->stops_at_route.cbegin();
			it_by_stops_at_bus != std::prev(it_by_buses->stops_at_route.cend(), 1);
			++it_by_stops_at_bus) {
			auto it_next_ = std::next(it_by_stops_at_bus, 1);

			auto it1 = stops_at_route.find((*it_by_stops_at_bus)->name);
			auto it2 = stops_at_route.find((*it_next_)->name);

			if ((it1 != stops_at_route.end() && it2 == stops_at_route.end()) || ((*it_by_stops_at_bus)->name == (*it_next_)->name)) {
				set_for_graph_.insert({ *(*it_by_stops_at_bus) ,true });
			}
			stops_at_route.insert((*it_by_stops_at_bus)->name);
			set_for_graph_.insert({ *(*it_by_stops_at_bus) ,false });
			set_for_graph_.insert({ *(*it_next_) ,false });
		}
	}
	graph_ = graph::DirectedWeightedGraph <Weight>(set_for_graph_.size());
	this->BuildEdge();
}

void RouterTransport::TransportRouter::BuildEdge()
{
	//для некругового маршрута
	for (auto it_by_buses = transport_cataloge_.BusesCBegin();
		it_by_buses != transport_cataloge_.BusesCEnd();
		++it_by_buses) {
		if (it_by_buses->circle) {
			continue;
		}
		for (auto it_by_stops_at_bus = it_by_buses->stops_at_route.cbegin();
			it_by_stops_at_bus != std::prev(it_by_buses->stops_at_route.cend(), 1);
			++it_by_stops_at_bus) {

			Weight prev_weight, prev_weight_reverse;
			for (auto next_it_by_stops_at_bus = std::next(it_by_stops_at_bus, 1);
				next_it_by_stops_at_bus != it_by_buses->stops_at_route.cend();
				++next_it_by_stops_at_bus) {
				auto it_first_at_set = set_for_graph_.find({ **it_by_stops_at_bus ,false });
				auto pos_first = std::distance(set_for_graph_.begin(), it_first_at_set);

				auto it_second_at_set = set_for_graph_.find({ **next_it_by_stops_at_bus ,false });
				auto pos_second = std::distance(set_for_graph_.begin(), it_second_at_set);
				//теоретически reverse тоже может быть емпти, не знаю нужно ли рассматривать такой случай.
				if (prev_weight.Empty()) {
					int distance = transport_cataloge_.GetDistance(*it_by_stops_at_bus, *next_it_by_stops_at_bus);
					double time = distance / rout_settings_.velocity;
					prev_weight = { time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,{1} };
					graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second), prev_weight });

					distance = transport_cataloge_.GetDistance(*next_it_by_stops_at_bus, *it_by_stops_at_bus);
					if (distance != 0) {
						if (!prev_weight_reverse.Empty()) {
							double time = distance / rout_settings_.velocity;
							prev_weight_reverse = { prev_weight_reverse.travel_time + time,rout_settings_.wait,it_by_buses->name,(*next_it_by_stops_at_bus)->name,(*it_by_stops_at_bus)->name,prev_weight_reverse.span_count + 1 };
							graph_.AddEdge({ (size_t)(pos_second), (size_t)(pos_first), prev_weight_reverse });
						}
						else {
							double time = distance / rout_settings_.velocity;
							prev_weight_reverse = { time,rout_settings_.wait,it_by_buses->name,(*next_it_by_stops_at_bus)->name,(*it_by_stops_at_bus)->name,{1} };
							graph_.AddEdge({ (size_t)(pos_second), (size_t)(pos_first), prev_weight_reverse });
						}
					}
				}
				else {
					int distance = transport_cataloge_.GetDistance(*std::prev(next_it_by_stops_at_bus, 1), *next_it_by_stops_at_bus);
					double time = distance / rout_settings_.velocity;
					prev_weight = { prev_weight.travel_time + time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,prev_weight.span_count + 1 };
					graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second), prev_weight });
					distance = transport_cataloge_.GetDistance(*next_it_by_stops_at_bus, *std::prev(next_it_by_stops_at_bus, 1));
					if (distance != 0) {
						if (!prev_weight_reverse.Empty()) {
							double time = distance / rout_settings_.velocity;
							prev_weight_reverse = { prev_weight_reverse.travel_time + time,rout_settings_.wait,it_by_buses->name,(*next_it_by_stops_at_bus)->name,(*it_by_stops_at_bus)->name,prev_weight_reverse.span_count + 1 };
							graph_.AddEdge({ (size_t)(pos_second), (size_t)(pos_first), prev_weight_reverse });
						}
						else {
							double time = distance / rout_settings_.velocity;
							prev_weight_reverse = { time,rout_settings_.wait,it_by_buses->name,(*next_it_by_stops_at_bus)->name,(*it_by_stops_at_bus)->name,{1} };
							graph_.AddEdge({ (size_t)(pos_second), (size_t)(pos_first), prev_weight_reverse });
						}
					}
				}
			}
		}
	}
	//для круга
	for (auto it_by_buses = transport_cataloge_.BusesCBegin();
		it_by_buses != transport_cataloge_.BusesCEnd();
		++it_by_buses) {
		if (!it_by_buses->circle) {
			continue;
		}
		for (auto it_by_stops_at_bus = it_by_buses->stops_at_route.cbegin();
			it_by_stops_at_bus != std::prev(it_by_buses->stops_at_route.cend(), 1);
			++it_by_stops_at_bus) {

			Weight prev_weight;
			for (auto next_it_by_stops_at_bus = std::next(it_by_stops_at_bus, 1);
				next_it_by_stops_at_bus != it_by_buses->stops_at_route.cend();
				++next_it_by_stops_at_bus) {
				//если связываем 2 разные остановки
				if ((*it_by_stops_at_bus)->name != (*next_it_by_stops_at_bus)->name ||
					((it_by_stops_at_bus == it_by_buses->stops_at_route.cbegin() &&
						next_it_by_stops_at_bus == std::prev(it_by_buses->stops_at_route.cend(), 1)) &&
						(*it_by_stops_at_bus)->name == (*next_it_by_stops_at_bus)->name)) {
					auto it_first_at_set = set_for_graph_.find({ **it_by_stops_at_bus ,false });
					auto pos_first = std::distance(set_for_graph_.begin(), it_first_at_set);

					auto it_second_at_set = set_for_graph_.find({ **next_it_by_stops_at_bus ,false });
					auto pos_second = std::distance(set_for_graph_.begin(), it_second_at_set);

					if (prev_weight.Empty()) {
						int distance = transport_cataloge_.GetDistance(*it_by_stops_at_bus, *next_it_by_stops_at_bus);
						double time = distance / rout_settings_.velocity;
						prev_weight = { time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,{1} };
						graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second), prev_weight });
					}
					else {
						int distance = transport_cataloge_.GetDistance(*std::prev(next_it_by_stops_at_bus, 1), *next_it_by_stops_at_bus);
						double time = distance / rout_settings_.velocity;
						prev_weight = { prev_weight.travel_time + time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,prev_weight.span_count + 1 };
						graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second), prev_weight });
					}

				}
				else {
					auto it_first_at_set = set_for_graph_.find({ **it_by_stops_at_bus ,false });
					auto pos_first = std::distance(set_for_graph_.begin(), it_first_at_set);

					auto it_second_at_set_not_transfer = set_for_graph_.find({ **next_it_by_stops_at_bus ,false });
					auto pos_second_not_transfer = std::distance(set_for_graph_.begin(), it_second_at_set_not_transfer);

					auto it_second_at_set_transfer = set_for_graph_.find({ **next_it_by_stops_at_bus ,true });
					auto pos_second_transfer = std::distance(set_for_graph_.begin(), it_second_at_set_transfer);
					//костыль, необходимый для каких-то рандомных данных, я не понимаю на каких данных это может лечь...
					if (it_second_at_set_transfer == set_for_graph_.end())
					{
						continue;
					}

					if (prev_weight.Empty()) {
						int distance = transport_cataloge_.GetDistance(*it_by_stops_at_bus, *next_it_by_stops_at_bus);
						double time = distance / rout_settings_.velocity;
						prev_weight = { time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,{1} };
						graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second_transfer), prev_weight });
						graph_.AddEdge({ (size_t)(pos_second_transfer), (size_t)(pos_second_not_transfer), {0,0,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,prev_weight.span_count} });
					}
					else {
						int distance = transport_cataloge_.GetDistance(*std::prev(next_it_by_stops_at_bus, 1), *next_it_by_stops_at_bus);
						double time = distance / rout_settings_.velocity;
						prev_weight = { prev_weight.travel_time + time,rout_settings_.wait,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,prev_weight.span_count + 1 };
						graph_.AddEdge({ (size_t)(pos_first), (size_t)(pos_second_transfer), prev_weight });
						graph_.AddEdge({ (size_t)(pos_second_transfer), (size_t)(pos_second_not_transfer), {0,0,it_by_buses->name,(*it_by_stops_at_bus)->name,(*next_it_by_stops_at_bus)->name,prev_weight.span_count} });

					}
				}
			}
		}
	}
	is_empty_ = false;
}

std::optional<std::vector<RouterTransport::TransportRouter::Weight>> RouterTransport::TransportRouter::RequestRoute(const RoutingSettings& rout_settings)
{

	static graph::Router router(graph_);
	auto ptr_stop_from = transport_cataloge_.GetStop(rout_settings.from_station);
	auto ptr_stop_to = transport_cataloge_.GetStop(rout_settings.to_station);
	if (ptr_stop_from == ptr_stop_to) {
		return  std::vector<Weight>{};
	}
	else if ((ptr_stop_from == nullptr || ptr_stop_to == nullptr) ||
		(ptr_stop_from->buses.empty() || ptr_stop_to->buses.empty())) {
		return {};
	}

	auto it_stop_from = set_for_graph_.find({ {ptr_stop_from->name,{},{}},false });
	auto it_stop_to = set_for_graph_.find({ {ptr_stop_to->name,{},{}},false });
	size_t pos_from = std::distance(set_for_graph_.begin(), it_stop_from);
	size_t pos_to = std::distance(set_for_graph_.begin(), it_stop_to);


	auto route = router.BuildRoute(pos_from, pos_to);
	std::vector<Weight> result;
	if (route)
	{
		for (const auto el : route->edges) {
			result.push_back(graph_.GetEdge(el).weight);
		}
	}
	else {
		return {};
	}
	return result;
}

bool RouterTransport::TransportRouter::Empty() const
{
	return is_empty_;
}
