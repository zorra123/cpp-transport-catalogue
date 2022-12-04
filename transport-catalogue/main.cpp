#include <iostream>
#include <string_view>
#include <vector>
#include <iostream>
#include <fstream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
using namespace std;


int main() {


	ifstream file("my.json");
	DataBase::TransportCatalogue catalog;
	Request::RequestHandler req(catalog);
	ReadJson(file, req);
	//ReadJson(std::cin, test);
	req.PrintAnswer(std::cout);
	return 0;
}