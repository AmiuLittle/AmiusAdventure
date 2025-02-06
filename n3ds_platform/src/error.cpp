#include "error.hpp"

std::string error = "";

void setErr(std::string err) {
    error = err;
}
std::string getErr() {
    return error;
}