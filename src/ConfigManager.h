//
//  ConfigManager.hpp
//  egg scramble
//
//  Created by Phoebe on 4/18/16.
//  Copyright © 2016 sunny side up. All rights reserved.
//
#pragma once

#include <string>
#include <map>

class ConfigManager
{
public:
    std::map<std::string, std::string> cfg_map;
    
    // use this to load the file into the manager
    void LoadConfigs(const std::string& file_name);

    // use this to get the value read from the config file
    static std::string GetConfigValue(const std::string& key);
    static int GetAsInt(const std::string& key);
    static long GetAsLong(const std::string& key);
    static float GetAsFloat(const std::string& key);

    static ConfigManager* instance()
    {
        static ConfigManager* instance = new ConfigManager();
        return instance;
    }

private:
    ConfigManager(void) {}
    ~ConfigManager(void) {}
    bool HasPrefix(const std::string& word, const std::string& prefix) const;
};
