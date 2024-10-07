#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include "data.hpp"
#include "config.hpp"
#include "ThreadPool.hpp"

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

            // 如果文件夹不存在则创建
            util fu1(_back_path);
            util fu2(_pack_path);
            if (!fu1.exists())
                fu1.createDirectory();
            if (!fu2.exists())
                fu2.createDirectory();
        }

        bool runModule()
        {
            ThreadPool *pool = ThreadPool::getInstance();

            // 将遍历到的文件进行管理，map中左边是文件名，右边是文件是否正在被压缩的状态，其目的是如果该文件处于被放入线程池中正在处理压缩就不继续放入线程池中
            std::unordered_map<std::string, bool> file_compress_status;

            while (true)
            {
                // 从存放文件的文件夹中读取所有文件名
                util fu(_back_path);
                std::vector<std::string> arr;
                fu.scanDirectory(&arr);

                // 将所有获取到的文件放入到哈希表中
                // 如果没有遍历到文件，则清空哈希表
                if (arr.empty())
                    file_compress_status.clear();
                for (auto &file : arr)
                {
                    file_compress_status.insert(std::pair<std::string, bool>(file, false));
                }

                // 根据文件名获取最后一次访问时间，判断是否是热点文件
                for (int i = 0; i < arr.size(); ++i)
                {
                    std::string file = arr[i];
                    // 是热点文件，不做处理
                    if (isHotFile(file))
                        continue;
                    BackupInfo finfo;

                    // 如果获取失败说明这个文件没有存备份信息，那就新增备份信息
                    if (!data->getOneByRealPath(file, &finfo))
                    {
                        finfo.createBackupInfo(file);
                    }

                    // 压缩文件使用线程池
                    if (!file_compress_status[file])
                    {
                        pool->enques(compressFile, file, finfo);
                        file_compress_status[file] = true; // 将是否被压缩标识设置为true
                    }
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

        static void compressFile(std::string file, BackupInfo finfo)
        {
            // 如果文件太大，压缩就会导致服务器挂掉，目前这里就不压缩,这里就设置如果文件大小大于30M就不压缩
            if (util(file).fileSize() > 31457280)
            {
                // std::cout << file << " 文件大小为:" << util(file).fileSize() << "文件太大，不压缩" << std::endl;
                return;
            }

            // 对非热点文件进行压缩
            util temp(file);
            temp.compress(finfo._pack_path);
            // 删除文件
            temp.rmdir();
            finfo._pack_flag = true; // 将是否压缩标识设置为true
            data->update(finfo);

            return;
        }

    private:
        std::string _back_path;   // 备份文件存放路径
        std::string _pack_path;   // 压缩文件路径
        std::string _pack_suffix; // 压缩文件后缀名
        int _hot_time;            // 用于判断是否是热点文件
    };
}