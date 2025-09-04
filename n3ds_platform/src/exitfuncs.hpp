#pragma once
#ifndef N3DS_BACKEND_PANIC
#include <string>

void unlockCore1();
void checkPanic();
void threadCheckPanic();
void exitGame();
void softPanic(std::string reason);

#define N3DS_BACKEND_PANIC
#endif