#pragma once
#include "geo.h"
#include "transport_catalogue.h"
#include <iostream>
#include <ostream>

void StatReader(std::istream& stream_input, std::ostream& stream_output, DataBase::TransportCatalogue& catalog);
