#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include "data.hpp"
#include "config.hpp"

extern cloud::DataManager *data;

namespace cloud
{
    class HotManager
    {
    public:
        HotManager()
        {
            cloud::Config *config = Config::getInstance();
            _back_path = config->getBackDir();
            _pack_path = config->getPackDir();
            _pack_suffix = config->getPackFileSuffix();
            _hot_time = config->getHotTime();

            //如果文件夹不存在则创建
            util fu1(_back_path);
            util fu2(_pack_path);
            if(!fu1.exists()) fu1.createDirectory();
            if(!fu2.exists()) fu2.createDirectory();
        }

        bool runModule()
        {
            while (true)
            {
                // 从存放文件的文件夹中读取所有文件名
                util fu(_back_path);
                std::vector<std::string> arr;
                fu.scanDirectory(&arr);

                // 根据文件名获取最后一次访问时间，判断是否是热点文件
                for (auto &file : arr)
                {
                    // 是热点文件，不做处理
                    if (isHotFile(file))
                        continue;
                    BackupInfo finfo;

                    // 如果获取失败说明这个文件没有存备份信息，那就新增备份信息
                    if (!data->getOneByRealPath(file, &finfo))
                    {
                        finfo.createBackupInfo(file);
                    }
                    // 对非热点文件进行压缩
                    util temp(file);
                    temp.compress(finfo._pack_path);
                    // 删除文件
                    temp.rmdir();
                    finfo._pack_flag = true; // 将是否压缩标识设置为true
                    data->update(finfo);
                }
                usleep(1000);
            }
        }

    private:
        // 判断是不是一个热点文件
        bool isHotFile(const std::string &filename)
        {
            // 获取上一次访问时间
            util fu(filename);
            time_t a_time = fu.lastAccTime();
            // 获取当前时间
            time_t cur_time = time(nullptr);
            // 热点文件返回真，非热点文件返回假
            return cur_time - a_time < _hot_time;
        }

    private:
        std::string _back_path;   // 备份文件存放路径
        std::string _pack_path;   // 压缩文件路径
        std::string _pack_suffix; // 压缩文件后缀名
        int _hot_time;            // 用于判断是否是热点文件
    };
}