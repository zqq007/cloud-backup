#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <filesystem>
#include "sys/stat.h"

namespace cloud
{
    namespace fs = std::filesystem;
    class util
    {
    public:
        util(const std::string& filename) : filename_(filename) {}

        size_t fileSize()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return 0;
            }
            return st.st_size;
        }

        time_t lastModTime()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cout << "get file last modify time failed" << std::endl;
                return -1;
            }
            return st.st_mtime;
        }

        time_t lastAccTime()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cout << "get file last access time failed" << std::endl;
                return -1;
            }
            return st.st_atime;
        }

        std::string fileName()
        {
            auto pos = filename_.find_last_of("/");
            if (pos == std::string::npos)
                return filename_;
            return filename_.substr(pos + 1);
        }

        // 获取文件指定位置 指定长度的数据
        bool getPosLen(std::string* body, size_t pos, size_t len)
        {
            size_t fsize = this->fileSize();
            if (pos + len > fsize)
            {
                std::cerr << "get file len is error" << std::endl;
                return false;
            }

            std::ifstream ifs;
            ifs.open(filename_, std::ios::binary);
            if (ifs.is_open() == false)
            {
                std::cerr << "open file failed" << std::endl;
                return false;
            }

            ifs.seekg(pos, std::ios::beg);
            body->resize(fsize);
            ifs.read(&(*body)[0], len);
            if (ifs.good() == false)
            {
                std::cerr << "get file content fail" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        // 读取文件数据
        bool getContent(std::string* body)
        {
            size_t fsize = this->fileSize();
            return getPosLen(body, 0, fsize);
        }

        // 向文件写入数据
        bool setContent(const std::string& body)
        {
            std::ofstream ofs;
            ofs.open(filename_, std::ios::binary);
            if (ofs.is_open() == false)
            {
                std::cerr << "write open file failed" << std::endl;
                return false;
            }
            ofs.write(&body[0], body.size());
            if (ofs.good() == false)
            {
                std::cerr << "write file failed" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        bool exists()
        {
            return fs::exists(filename_);
        }

        bool createDirectory()
        {
            if (this->exists())
                return true;
            return fs::create_directories(filename_);
        }

        bool scanDirectory(std::vector<std::string>* array)
        {
            createDirectory();
            for (auto& p : fs::directory_iterator(filename_))
            {
                // 如果是文件夹就跳过
                if (fs::is_directory(p) == false)
                {
                    // relative_path 带有路径的文件名称
                    array->push_back(fs::path(p).relative_path().string());
                }
            }
            return true;
        }

        bool rmdir()
        {
            if (exists() == false)
                return false;
            remove(filename_.c_str());
            return true;
        }

    private:
        std::string filename_;
    };
}
