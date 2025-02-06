#pragma once
#ifndef N3DS_BACKEND_N3DSLINK

/// @brief Starts SOC service to print to 3dslink server
/// @return true if success, false if failure
bool init3dslinkStdio();

void exit3dslink();

#define N3DS_BACKEND_N3DSLINK
#endif