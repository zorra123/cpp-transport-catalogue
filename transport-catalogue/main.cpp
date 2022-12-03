#include "transport_catalogue.h"
#include "json_reader.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;


int main() {


	//ifstream file("my.json");
	DataBase::TransportCatalogue test;
	//ReadInputData(file, test);
	ReadInputData(std::cin, test);
	return 0;
}