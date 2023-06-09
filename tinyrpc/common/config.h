#pragma once 

#include <map>
#include <string>   

namespace tinyrpc {

class Config {
public:
    Config(const char* xml_file);

public:
    static Config* GetGlobalConfig();
    static void SetGlobalConfigPath(const char* xml_file);

public:
    std::string log_level_;

};

}   // namespace tinyrpc