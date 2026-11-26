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
    // パス定数（定義は conf.cpp）
    static const std::string CONFIG_DIR;
    static const std::string BOOT_PATH;
    static const std::string LIVE_PATH;

    ConfData params;

    // params の各メンバーへの参照を保持するため、コピー・ムーブは禁止する
    std::map<std::string, std::reference_wrapper<int>> registry;

    time_t lastLoadedTime;

    void setupBindings();
    static std::string trim(const std::string& s);
    static time_t getFileTimestamp(const std::string& path);
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path, const std::string& header);

public:
    ConfigManager();

    // registry が自身の params を参照するため、コピー・ムーブすると参照が壊れる
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    // 設定データへのアクセッサ
    const ConfData& get() const;

    bool checkAndReload();
};
