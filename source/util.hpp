#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <experimental/filesystem>
#include <jsoncpp/json/json.h>
#include "sys/stat.h"
#include "bundle.h"
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace cloud
{
    namespace fs = std::experimental::filesystem;
    class util
    {
    public:
        util(const std::string &filename) : filename_(filename) {}

        int64_t fileSize()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return -1;
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
        bool getPosLen(std::string *body, size_t pos, size_t len)
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
        bool getContent(std::string *body)
        {
            // size_t fsize = this->fileSize();
            // return getPosLen(body, 0, fsize);

            body->clear();
            struct stat s;
            int n = stat(filename_.c_str(), &s);
            if (n < 0)
                return false;

            int size = s.st_size;
            body->resize(size);

            int fd = open(filename_.c_str(), O_RDONLY);
            if (fd < 0)
                return false;

            // read(fd, const_cast<char *>(buff->c_str()), size);
            read(fd, &(*body)[0], size);

            close(fd);
            return true;
        }

        // 向文件写入数据
        bool setContent(const std::string &body)
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

        bool compress(const std::string &packname)
        {
            // 获取文件数据
            std::string body;
            if (this->getContent(&body) == false)
            {
                std::cerr << "comppress get file content failed" << std::endl;
                return false;
            }

            // 压缩
            std::string packed = bundle::pack(bundle::LZIP, body);

            // 将压缩后的数据存储到压缩包中
            cloud::util ut(packname);
            if (ut.setContent(packed) == false)
            {
                std::cerr << "write file error after compress" << std::endl;
                return false;
            }
            return true;
        }

        bool uncompress(const std::string &filename)
        {
            // 获取压缩文件的数据
            std::string body;
            if (this->getContent(&body) == false)
            {
                std::cerr << "uncomppress get file content failed" << std::endl;
                return false;
            }

            // 将数据进行解压
            std::string unpacked = bundle::unpack(body);

            // 将解压缩后的数据放到一个新的文件中
            cloud::util ut(filename);
            if (ut.setContent(unpacked) == false)
            {
                std::cerr << "uncompress write packed data failed" << std::endl;
                return false;
            }
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

        bool scanDirectory(std::vector<std::string> *array)
        {
            for (auto &p : fs::directory_iterator(filename_))
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

    class JsonUtil
    {
    public:
        static bool serialize(const Json::Value &root, std::string *str)
        {
            Json::StreamWriterBuilder swb;
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
            std::stringstream ss;
            if (sw->write(root, &ss))
            {
                std::cerr << "json write failed" << std::endl;
                return false;
            }
            *str = ss.str();
            return true;
        }

        static bool unserialize(const std::string &str, Json::Value *root)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            std::string err;
            bool res = cr->parse(&str[0], &str[str.size()], root, &err);
            if (res == false)
            {
                std::cerr << "parse err:" << err << std::endl;
                return false;
            }
            return true;
        }
    };

    // class CharacterConversionUitl
    // {
    // public:
    //     static std::string UnicodeToUTF8(const std::string &unicode_str)
    //     {
    //         std::string utf8_str;
    //         std::istringstream iss(unicode_str);
    //         char c;

    //         while (iss >> std::noskipws >> c)
    //         {
    //             if (c == '\\')
    //             {
    //                 char next = iss.peek();
    //                 if (next == 'u')
    //                 {
    //                     iss.get(); // consume 'u'
    //                     unsigned int code_point;
    //                     iss >> std::hex >> code_point;

    //                     // Convert code_point to UTF-8
    //                     if (code_point <= 0x7F)
    //                     {
    //                         utf8_str.push_back(static_cast<char>(code_point));
    //                     }
    //                     else if (code_point <= 0x7FF)
    //                     {
    //                         utf8_str.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    //                     }
    //                     else if (code_point <= 0xFFFF)
    //                     {
    //                         utf8_str.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    //                     }
    //                     else if (code_point <= 0x10FFFF)
    //                     {
    //                         utf8_str.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    //                         utf8_str.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    //                     }
    //                     else
    //                     {
    //                         // throw std::runtime_error("Invalid Unicode code point");
    //                         return "";//直接返回空
    //                     }
    //                 }
    //                 else
    //                 {
    //                     // throw std::runtime_error("Expected '\\u' sequence");
    //                     return "";
    //                 }
    //             }
    //             else
    //             {
    //                 utf8_str.push_back(c);
    //             }
    //         }

    //         return utf8_str;
    //     }
    // };
}