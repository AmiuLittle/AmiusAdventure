#include "error.hpp"

std::string error = "All good :3";

void setErr(std::string err) {
    error = err;
}
std::string getErr() {
    return error;
}