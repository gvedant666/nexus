#include "system_utils.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <thread>
#include <chrono>

namespace SystemUtils {

    std::string executeAndCapture(const std::string& cmd) {
        char buffer[128];
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }
        return result;
    }

    bool executeCommand(const std::string& cmd) {
        int status = std::system(cmd.c_str());
        return (status == 0);
    }

    std::string findVirtualWirelessDevice() {
        // Exclude phy#0 because that my Wi-Fi
        std::string output = SystemUtils::executeAndCapture("iw dev | grep phy# | grep -v 'phy#0' | head -n 1");
        if (output.empty()) return "";

        if (const size_t pos = output.find('#'); pos != std::string::npos) {
            std::string num = output.substr(pos + 1);
            num.erase(num.find_last_not_of(" \n\r\t")+1);
            return "phy" + num;
        }
        return "";
    }
}