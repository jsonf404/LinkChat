#pragma once

#include "const.h"
/*
 * SectionInfo：表示一个 [section]
 * 仅提供只读接口，避免外部随意修改配置
 */
class SectionInfo {
public:
    SectionInfo() = default;

    // 是否存在 key
    bool has(const std::string& key) const;

    // 获取 value，不存在返回空字符串引用
    const std::string& get(const std::string& key) const;

private:
    friend class ConfigMgr; // 只允许 ConfigMgr 写数据
    std::map<std::string, std::string> _section_datas;
};

/*
 * ConfigMgr：配置管理器
 */
class ConfigMgr {
public:
    ConfigMgr();
    // 禁止拷贝，配置管理器应唯一
    ConfigMgr(const ConfigMgr&) = delete;
    ConfigMgr& operator=(const ConfigMgr&) = delete;

    // 获取 section（只读）
    const SectionInfo& getSection(const std::string& section) const;

    // 是否存在 section
    bool hasSection(const std::string& section) const;

    static ConfigMgr& instance()//增加实例获取接口
    {
        static ConfigMgr inst;
        return inst;
    }

private:
    void loadConfig(const std::string& filePath);
    std::map<std::string, SectionInfo> _config_map;
};
