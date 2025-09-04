#pragma once
#ifndef N3DS_PLATFORM_TESTS_LOG_TO_SDMC
#include <string>

bool initSdmcLogger();
bool initSdmcLogger(std::string);
bool sdmcLog(std::string);
bool sdmcLogLn();
bool sdmcLogLn(std::string);
void exitSdmcLogger();

#define N3DS_PLATFORM_TESTS_LOG_TO_SDMC
#endif