#include "input_reader.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <string_view>
#include <list>

using namespace std::string_literals;
using namespace DataBase;

//ReadInputData(функция) вызывается от переданного потока ввода и класса базы, там создается класс, закрытый извне. В этом классе, 
//сначала в конструкторе происходит заполнение Stpos,а для Bus создается временный list структр Bus.
//В деструторе будем класть Bus в базу данных.
//
class ReadingInputData {
private:
	struct Bus {
		std::string num;
		std::vector<std::string> name_stops;
		bool flag_is_cirle_route;
	};
public:
	//конструктор, тут читаются все остановки и заполняется база по ним, заполнение Bus будет в деструторе
	ReadingInputData(std::istream& stream, TransportCatalogue& catalog) :catalog_(catalog) {
		std::string num;
		getline(stream, num);
		for (int i = 0; i < std::stoi(num); ++i) {
			std::string str;
			getline(stream, str);
			auto res = SplitIntoWordsViewNew(str);
			if (!res.empty()) {
				TransportCatalogue::Stop stop;
				stop.name = res[1];
				stop.coords = { std::stod(std::string{res[2]}), std::stod(std::string{res[3]}) };
				catalog.AddStop(stop);
			}

		}
	}
	~ReadingInputData() {
		catalog_.InputAllDistance(distance_to_another_stops);
		for (auto& el : bus_str_) {
			catalog_.AddBus(el.num, el.name_stops, el.flag_is_cirle_route);
		}
	}
private:
	TransportCatalogue& catalog_;
	std::list<Bus> bus_str_;
	std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> distance_to_another_stops;

	std::vector<std::string_view> SplitIntoWordsViewNew(std::string_view str) {
		std::vector<std::string_view> result;
		std::string_view word = str.substr(0, str.find_first_of(' '));
		str.remove_prefix(str.find_first_of(' '));
		if (word == "Stop"s) {
			result.push_back(word);
			//находим название, даже если оно состоит из n слов
			auto word_begin = str.find_first_not_of(' ');
			auto word_end = str.find_first_of(':');
			word = str.substr(word_begin, word_end - word_begin);
			result.push_back(word);
			str.remove_prefix(word_end + 1);

			//находим 1ю координату
			word_begin = str.find_first_not_of(' ');
			word_end = str.find_first_of(',');
			word = str.substr(word_begin, word_end - word_begin);
			result.push_back(word);
			str.remove_prefix(word_end + 1);

			//2ю
			word_begin = str.find_first_not_of(' ');
			word_end = str.find_first_of(',');
			if (word_end == str.npos) {//если не найдена, то мы имеем только координаты без указания расстояния до других остановок
				word = str.substr(word_begin, str.size() - word_begin);
				result.push_back(word);
			}
			else {
				word = str.substr(word_begin, word_end - word_begin);
				result.push_back(word);
				str.remove_prefix(word_end + 1);// коордианыт взяты, str  содержит ' ' и оставшуюся информацию
				while (!str.empty()) {//здесь происходит оставшаяся обработка
					word_begin = str.find_first_not_of(' ');
					str.find_first_of(',') != str.npos ?
						word_end = str.find_first_of(',') :
						word_end = str.size();

						word = str.substr(word_begin, word_end- word_begin);
						auto this_word_end = str.find_first_of('m');
						int distance = std::stoi(std::string{ word.substr(0,this_word_end) });
						word.remove_prefix(this_word_end + 1);

						auto tmp = word.find("to ");
						size_t size_razdelitel_to = 3;
						word.remove_prefix(tmp+ size_razdelitel_to);
						distance_to_another_stops[std::string{ result[1] }].push_back({ std::string{word}, distance });
						str.remove_prefix(word_end + 1>str.size()? str.size(): word_end + 1);
				}
			}
		}
		else if (word == "Bus"s) {
			Bus bus;
			//находим номер автобуса
			auto word_begin = str.find_first_not_of(' ');
			auto word_end = str.find_first_of(':');
			word = str.substr(word_begin, word_end - word_begin);
			bus.num = std::string{ word };
			str.remove_prefix(word_end + 1);
			//после строчки выше, в str лежит ' 'и оставшаяся строка
			char razdelitel;
			str.find('-') != str.npos ? razdelitel = '-' : razdelitel = '>';
			while (word_end != str.npos) {
				word_begin = str.find_first_not_of(' ');
				word_end = str.find_first_of(razdelitel);
				word_end != str.npos ? word = str.substr(word_begin, word_end - word_begin - 1) : word = str.substr(word_begin, str.size() - word_begin);
				bus.name_stops.push_back(std::string{ word });
				str.remove_prefix(word_end + 1);
			}
			razdelitel == '>' ? bus.flag_is_cirle_route = true : bus.flag_is_cirle_route = false;
			bus_str_.push_back(std::move(bus));
		}
		return result;
	}
};
void ReadInputData(std::istream& stream, TransportCatalogue& catalog)
{
	ReadingInputData mydata(stream, catalog);
}
