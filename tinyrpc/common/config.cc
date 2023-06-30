#include <tinyxml/tinyxml.h>

#include "tinyrpc/common/config.h"

#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if (!name##_node) { \
    printf("Start tinyRPC server error, failed to read node [%s]", #name); \
    exit(0); \
} \

#define READ_STR_FROM_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if (!name##_node || !name##_node->GetText()) { \
    printf("Start tinyRPC server error, failed to read config file %s", #name); \
    exit(0); \
} \
std::string name##_str = name##_node->GetText(); \


namespace tinyrpc {

static Config* global_config = nullptr;

Config* Config::GetGlobalConfig() {
    return global_config;
}

void Config::SetGlobalConfigPath(const char* xml_file) {
    if (global_config == nullptr) {
        global_config = new Config(xml_file);
    } else {
        printf("Start tinyRPC server error, global config has been set");
        exit(0);
    }
        
}

Config::Config(const char* xml_file) {
    TiXmlDocument* xml_document = new TiXmlDocument();

    bool load_ok = xml_document->LoadFile(xml_file);
    if (!load_ok) {
        printf("Start tinyRPC server error, failed to read config file %s, error info[%s] \n", xml_file, xml_document->ErrorDesc());
        exit(0);
    }

    READ_XML_NODE(root, xml_document);
    READ_XML_NODE(log, root_node);

    READ_STR_FROM_XML_NODE(log_level, log_node);

    log_level_ = log_level_str;
}

}   // namespace tinyrpc