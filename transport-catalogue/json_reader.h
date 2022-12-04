#pragma once
#include "transport_catalogue.h"
#include "request_handler.h"
#include <iostream>

void ReadJson(std::istream& stream, /*DataBase::TransportCatalogue& catalog,*/ Request::RequestHandler& req);
