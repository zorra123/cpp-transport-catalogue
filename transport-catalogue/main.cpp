#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;


int main() {

	/*ifstream file("tsA_case2_input.txt");
	TransportCatalogue test;
	ReadInputData(file, test);
	StatReader(file, test);*/
	DataBase::TransportCatalogue test;
	ReadInputData(cin, test);
	StatReader(cin, cout, test);
	return 0;
}