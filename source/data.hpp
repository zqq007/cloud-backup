#pragma once

#include <iostream>
#include <pthread.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "util.hpp"
#include "config.hpp"

namespace cloud
{
    typedef struct BackupInfo
    {
        bool _pack_flag;        // 是否压缩标识符
        size_t _fsize;          // 文件大小
        time_t _atime;          // 最后一次访问时间
        time_t _mtime;          // 最后一次修改时间
        std::string _real_path; // 文件实际存放路径
        std::string _pack_path; // 压缩包管理路径
        std::string _url_path;

        bool createBackupInfo(const std::string &realpath)
        {
            cloud::Config *config = cloud::Config::getInstance();
            // std::cout << realpath << std::endl;

            // 获取文件信息
            util fu(realpath);
            // if (fu.exists() == false)
            // {
            //     std::cout << "file not exists" << std::endl;
            //     return false;
            // }

            _pack_flag = false;
            _fsize = fu.fileSize();
            _atime = fu.lastAccTime();
            _mtime = fu.lastModTime();
            _real_path = realpath;
            /*这里需要注意一下：压缩包管理路径
            客户上传的是文件存放的路径是"./backdir/"
            而我们的压缩管理路径"./packdir/"，并且压缩后的文件名还要加上后缀：".lz"
            */
            // 压缩包路径前缀
            std::string file_prefix = config->getPackDir();
            // 压缩包名后缀
            std::string file_suffix = config->getPackFileSuffix();
            // 整合起来就是压缩包管理路径
            _pack_path = file_prefix + fu.fileName() + file_suffix;

            /*和上面一样，假如客户上传的文件整体路径是：./backdir/a.txt
            下一次客户想再下载的话，就得是url，对应类内成员_url_path*/

            std::string download_prefix = config->getDownloadPrefix();
            _url_path = download_prefix + fu.fileName();

            return true;
        }

    } BackupInfo;

    class DataManager
    {
    public:
        DataManager()
        {
            _backup_file = Config::getInstance()->getBackupFile();
            pthread_rwlock_init(&_lock, nullptr);
            initLoad();
        }

        // 初始化程序运行时从文件读取数据
        bool initLoad()
        {
            // 从文件中把数据读出来
            util fu(_backup_file);
            // 读数据之前先判断文件存不存在，如果不存在说明从来没有存过数据
            if (fu.exists() == false)
                return true;
            std::string body;
            fu.getContent(&body);
            // 对读到的数据进行反序列化
            Json::Value root;
            if (!JsonUtil::unserialize(body, &root))
            {
                std::cerr << "unserialize fail" << std::endl;
                return false;
            }
            // 最后填充到_tables中
            for (auto &it : root)
            {
                BackupInfo info;
                info._atime = it["atime"].asInt();
                info._fsize = it["fsize"].asInt();
                info._mtime = it["mtime"].asInt();
                info._pack_flag = it["pack_flag"].asBool();

                info._pack_path = it["pack_path"].asString();
                info._real_path = it["real_path"].asString();
                info._url_path = it["url"].asString();
                insert(info);
            }

            return true;
        }

        // 信息发生改变就持久化存储一次
        bool storage()
        {
            // 获取所有数据
            std::vector<BackupInfo> arr;
            getAll(&arr);

            // 对所有数据信息以json格式存储
            Json::Value root;
            for (auto &it : arr)
            {
                Json::Value item;
                item["atime"] = (Json::Int64)it._atime;
                item["fsize"] = (Json::Int64)it._fsize;
                item["mtime"] = (Json::Int64)it._mtime;
                item["pack_flag"] = it._pack_flag;
                item["pack_path"] = it._pack_path;
                item["real_path"] = it._real_path;
                item["url"] = it._url_path;
                root.append(item);
            }

            // 对Json进行序列化
            std::string body;
            if (!JsonUtil::serialize(root, &body))
            {
                std::cerr << "serialize fail" << std::endl;
                return false;
            }

            // 将Json写到文件中去
            util fu(_backup_file);
            fu.setContent(body);
            return true;
        }

        bool insert(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&_lock);
            _tables[info._url_path] = info;
            pthread_rwlock_unlock(&_lock);

            // 新增就持久化存储一次
            storage();

            return true;
        }

        bool update(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&_lock);
            _tables[info._url_path] = info;
            pthread_rwlock_unlock(&_lock);

            // 更新也需要持久化
            storage();

            return true;
        }

        // 通过url获取文件信息
        bool getOneByURL(const std::string &url, BackupInfo *info)
        {
            // 由于_tables本身就是以文件url为key值，所以直接访问就行
            pthread_rwlock_wrlock(&_lock);
            // 首先判断这个url存不存在
            std::unordered_map<std::string, BackupInfo>::iterator it = _tables.find(url);
            if (it == _tables.end())
            {
                std::cout << "not find" << std::endl;
                pthread_rwlock_unlock(&_lock);
                return false;
            }
            *info = _tables[url];
            pthread_rwlock_unlock(&_lock);
            return true;
        }

        // 通过文件实际存放路径获取文件信息
        bool getOneByRealPath(const std::string &realpath, BackupInfo *info)
        {
            pthread_rwlock_wrlock(&_lock);
            // 循环遍历
            std::unordered_map<std::string, BackupInfo>::iterator it = _tables.begin();
            while (it != _tables.end())
            {
                if (it->second._real_path == realpath)
                {
                    *info = it->second;
                    pthread_rwlock_unlock(&_lock);
                    return true;
                }
                it++;
            }
            pthread_rwlock_unlock(&_lock);
            return false;
        }

        // 获取所有文件信息
        bool getAll(std::vector<BackupInfo> *arr)
        {
            pthread_rwlock_wrlock(&_lock);
            // 循环遍历
            std::unordered_map<std::string, BackupInfo>::iterator it = _tables.begin();
            while (it != _tables.end())
            {
                arr->push_back(it->second);
                it++;
            }
            pthread_rwlock_unlock(&_lock);
            return true;
        }

        ~DataManager() { pthread_rwlock_destroy(&_lock); }

    private:
        std::string _backup_file;                            // 对文件信息做持久化
        std::unordered_map<std::string, BackupInfo> _tables; // 以文件访问url为key，文件信息做value
        pthread_rwlock_t _lock;
    };

}
