#include "ConfigMgr.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <iostream>

// ---------- SectionInfo 实现 ----------

static const std::string EMPTY_STRING;

bool SectionInfo::has(const std::string& key) const {
    return _section_datas.find(key) != _section_datas.end();
}

const std::string& SectionInfo::get(const std::string& key) const {
    auto it = _section_datas.find(key);
    if (it == _section_datas.end()) {
        return EMPTY_STRING;
    }
    return it->second;
}

// ---------- ConfigMgr 实现 ----------

static SectionInfo EMPTY_SECTION;

ConfigMgr::ConfigMgr() {
    // 当前工作目录 / config.ini
    boost::filesystem::path cfgPath =
        boost::filesystem::current_path() / "config.ini";

    loadConfig(cfgPath.string());
}

void ConfigMgr::loadConfig(const std::string& filePath) {
    boost::property_tree::ptree pt;

    try {
        boost::property_tree::read_ini(filePath, pt);
    }
    catch (const std::exception& e) {
        std::cerr << "[ConfigMgr] read config failed: "
            << e.what() << std::endl;
        return;
    }

    for (const auto& sectionPair : pt) {
        SectionInfo section;

        for (const auto& kv : sectionPair.second) {
            section._section_datas.emplace(
                kv.first,
                kv.second.get_value<std::string>());
        }

        _config_map.emplace(sectionPair.first, std::move(section));
    }
}

const SectionInfo& ConfigMgr::getSection(const std::string& section) const {
    auto it = _config_map.find(section);
    if (it == _config_map.end()) 
    {
        return EMPTY_SECTION;
    }
    return it->second;
}

bool ConfigMgr::hasSection(const std::string& section) const {
    return _config_map.find(section) != _config_map.end();
}
