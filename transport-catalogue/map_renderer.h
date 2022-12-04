#pragma once
#include <memory>
#include <vector>
#include <variant>
#include <string>
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"

/*
	��������� Settings - ��������� ��������� ��� svg. ����������� � ����� ������ � json_reader'e, �������� � �������  Rendering.
	� ������� Renderin ���������� ���������� � ������ ������� ���� �������� ��� svg ���������.
	������, ��� ����� .cpp � �� ������ �������������� .h � .cpp
*/

namespace map_reader {
	//
	enum Position {
		underlayer_color,
		color_palette
	};
	struct Settings {
		double width;
		double height;
		double padding;
		double line_width;
		double stop_radius;
		int bus_label_font_size;
		std::vector<double>bus_label_offset;
		int stop_label_font_size;
		std::vector<double>stop_label_offset;
		std::variant<std::string, svg::Rgb, svg::Rgba>underlayer_color;
		double underlayer_width;
		std::vector< std::variant<std::string, svg::Rgb, svg::Rgba>>color_palette;

		template <typename T>
		void SetColor(T color, Position pos) {
			if (pos == Position::underlayer_color) {
				underlayer_color = color;
			}
			else {
				color_palette.push_back(color);
			}
		}
	};



	std::stringstream Render(Settings& settings, DataBase::TransportCatalogue& catalog);
}