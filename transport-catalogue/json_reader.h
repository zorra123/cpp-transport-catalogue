#pragma once
#include <iostream>
#include "transport_catalogue.h"
#include "request_handler.h"

void ReadJson(std::istream& stream, Request::RequestHandler& req);
