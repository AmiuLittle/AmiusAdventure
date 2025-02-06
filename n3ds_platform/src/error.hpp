#pragma once
#ifndef N3DS_BACKEND_ERROR
#include <string>

void setErr(std::string);
std::string getErr();

#define N3DS_BACKEND_ERROR
#endif