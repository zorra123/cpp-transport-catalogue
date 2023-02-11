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
	
	/*ifstream file("my.json"s);
	ofstream file2;
	file2.open("out.json"s);	
	if (file.is_open() && file2.is_open()) {
		DataBase::TransportCatalogue catalog;
		Request::RequestHandler req(catalog);
		ReadJson(file, req);
		//ReadJson(std::cin, req);
		req.PrintAnswer(file2);
	}
	else { std::cout << "files not open" << std::endl; }*/
	
	DataBase::TransportCatalogue catalog;
	Request::RequestHandler req(catalog);
	ReadJson(std::cin, req);
	req.PrintAnswer(std::cout);
	return 0;
}
