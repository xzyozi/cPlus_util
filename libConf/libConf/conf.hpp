#pragma once

#include <string>
#include <map>
#include <ctime>
#include <functional> // std::reference_wrapper用
#include "confData.hpp"

//////////////////////////////////////////////////////////////////////////////////
// コンフィグマネージャー
//////////////////////////////////////////////////////////////////////////////////
class ConfigManager {
private:
    const std::string CONFIG_DIR = "config";
    const std::string BOOT_PATH = "config/boot.conf";
    const std::string LIVE_PATH = "config/live.conf";

    ConfData params;

    std::map<std::string, std::reference_wrapper<int>> registry;

    time_t lastLoadedTime;

    void setupBindings(); 
    std::string trim(const std::string& s);
    time_t getFileTimestamp(const std::string& path);
    bool loadFromFile(const std::string& path);
    void saveToFile(const std::string& path, const std::string& header);

public:
    ConfigManager();

    // 設定データへのアクセッサ
    const ConfData& get() const;

    bool checkAndReload();
};