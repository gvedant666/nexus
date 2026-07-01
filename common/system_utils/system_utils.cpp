#include "system_utils.h"
#include "UniqueFileLock.h"
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

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

    UniqueFileLock acquireVirtualWirelessDevice() {
        // Exclude phy#0 because that my Wi-Fi
        const std::string output = SystemUtils::executeAndCapture("iw dev | grep phy# | grep -v 'phy#0'");
        if (output.empty()) return {};

        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            const size_t pos = line.find('#');
            if (pos == std::string::npos) continue;

            std::string num = line.substr(pos + 1);
            num.erase(num.find_last_not_of(" \n\r\t") + 1);
            std::string phyName = "phy" + num;

            // Atomically claim this phy so two processes can never grab the same one
            std::string lockPath = "/tmp/p2p_lock_" + phyName;
            int fd = open(lockPath.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);
            if (fd < 0) continue; // already claimed, try next
            close(fd);
            return {phyName, lockPath};
        }
        return {};
    }
}
