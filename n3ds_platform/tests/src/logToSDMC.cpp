#include "logToSDMC.hpp"
#include <filesystem>
#include <fstream>
#include "error.hpp"

bool present = false;
std::ofstream output;

bool initSdmcLogger() {
    return initSdmcLogger("/amius_adventure_logs/log.txt");
}

bool initSdmcLogger(std::string strPath) {
    std::filesystem::path path(strPath);
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directories(path.parent_path());
    }

    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    output = std::ofstream(path);

    if (!output.is_open()) {
        setErr("failed to open output file: " + strPath);
        return false;
    }

    return true;
}
bool sdmcLog(std::string message) {
    if (present || !output.is_open()) {
        setErr("sdmcLog is not initialized");
        return false;
    }
    output << message;
    return true;
}
bool sdmcLogLn() {
    if (present || !output.is_open()) {
        setErr("sdmcLog is not initialized");
        return false;
    }
    output << std::endl;
    return true;
}
bool sdmcLogLn(std::string message) {
    sdmcLog(message);
    return sdmcLogLn();
}
void exitSdmcLogger() {
    if (present) {
        output.close();
    }
}