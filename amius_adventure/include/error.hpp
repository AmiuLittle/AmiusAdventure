#pragma once
#ifndef AMIUS_ADVENTURE_ERROR
#include <string>

void setErr(std::string);
std::string getErr();

#define AMIUS_ADVENTURE_ERROR
#endif