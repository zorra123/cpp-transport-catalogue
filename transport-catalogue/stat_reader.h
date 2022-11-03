#pragma once
#include "geo.h"
#include "transport_catalogue.h"
#include <iostream>

void StatReader(std::istream& stream, DataBase::TransportCatalogue& catalog);
