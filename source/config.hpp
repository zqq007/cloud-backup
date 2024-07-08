#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <filesystem>
#include "util.hpp"

#define CONFIG_FILE "./cloud.conf"

namespace cloud
{
    class Config
    {
    public:
        static Config *getInstance()
        {
            if (_instance == nullptr)
            {
                _mutex.lock();
                if (_instance == nullptr)
                {
                    _instance = new Config();
                }
                _mutex.unlock();
            }
            return _instance;
        }

        int getHotTime() { return _hot_time; }

        int getServerPort() { return _server_port; }

        std::string getServerIP() { return _server_ip; }

        std::string getDownloadPrefix() { return _download_prefix; }

        std::string getPackFileSuffix() { return _packfile_suffix; }

        std::string getPackDir()
        {
            if (!fs::exists(_pack_dir))
                fs::create_directory(_pack_dir);
            return _pack_dir;
        }

        std::string getBackDir()
        {
            if (!fs::exists(_back_dir))
                fs::create_directory(_back_dir);
            return _back_dir;
        }

        std::string getBackupFile()
        {
            return _backup_file;
        }

    private:
        int _hot_time;
        int _server_port;
        std::string _server_ip;
        std::string _download_prefix;
        std::string _packfile_suffix;
        std::string _pack_dir;
        std::string _back_dir;
        std::string _backup_file;

        bool readConfigFile()
        {
            util fu(CONFIG_FILE);
            std::string body;
            if (!fu.getContent(&body))
            {
                std::cerr << "load config file failed" << std::endl;
                return false;
            }

            // 拿到数据进行反序列化
            Json::Value root;
            if (!JsonUtil::unserialize(body, &root))
            {
                std::cerr << "unserialize failed" << std::endl;
                return false;
            }

            // 反序列化完成
            _hot_time = root["hot_time"].asInt();
            _server_port = root["server_port"].asInt();
            _server_ip = root["server_ip"].asString();
            _download_prefix = root["download_prefix"].asString();
            _packfile_suffix = root["packfile_suffix"].asString();
            _pack_dir = root["pack_dir"].asString();
            _back_dir = root["back_dir"].asString();
            _backup_file = root["backup_file"].asString();

            return true;
        }

    private:
        Config() { readConfigFile(); }
        static std::mutex _mutex;
        static Config *_instance;
    };

    Config *Config::_instance = nullptr;
    std::mutex Config::_mutex;
}