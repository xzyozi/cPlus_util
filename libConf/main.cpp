#include "header.h"
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include <thread>
#include <chrono>

int main() {

    ConfigManager conf;

    std::cout << "Retry Limit: " << conf.get().retry_limit << std::endl;
    std::cout << "Debug Mode : " << (conf.get().log_level ? "ON" : "OFF") << std::endl;

    int counter = 0;
    while (true) {
        // setting cheack
        if (conf.checkAndReload()) {
            std::cout << "-> Config Updated! New Retry Limit: " << conf.get().retry_limit << std::endl;
        }

        // --- 設定値を使用した処理 ---
        if (conf.get().exec_mode) {
             std::cout << "\rProcessing... " << counter++ 
                       << " (Limit: " << conf.get().retry_limit << ")" << std::flush;
        } else {
            std::cout << "\r[PAUSED] exec_mode is 0... " << std::flush;
        }


        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    std::cout << "Exited gracefully.\n";
    return 0;
}
