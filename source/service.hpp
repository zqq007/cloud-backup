#pragma once

#include <iostream>
#include <string>
#include "httplib.h"
#include "config.hpp"
#include "data.hpp"

extern cloud::DataManager *data;

namespace cloud
{
    class Service
    {
    public:
        Service()
        {
            Config *config = Config::getInstance();
            _server_port = config->getServerPort();
            _server_ip = config->getServerIP();
            _download_prefix = config->getDownloadPrefix();
        }

        bool runModule()
        {
            _server.Post("/upload", upLoad);
            _server.Get("/listshow", listShow);
            _server.Get("/", listShow); // 默认是展示界面

            // 获取download url
            std::string download_url = _download_prefix + "(.*)";
            _server.Get(download_url, download);

            _server.listen(_server_ip.c_str(), _server_port);

            return true;
        }

    private:
        static void upLoad(const httplib::Request &req, httplib::Response &res)
        {
            // 判断有没有文件上传
            bool ret = req.has_file("file");
            if (!ret)
            {
                res.status = 400;
                return;
            }

            // 获取文件信息
            const auto &file = req.get_file_value("file");
            std::string back_dir = Config::getInstance()->getBackDir();
            std::string real_path = back_dir + util(file.filename).fileName();
            util fu(real_path);

            // 将数据写入文件中
            fu.setContent(file.content);

            // 组织备份信息，并添加到数据管理模块中
            cloud::BackupInfo info;
            info.createBackupInfo(real_path);
            data->insert(info);

            return;
        }

        static std::string converter(time_t time)
        {
            std::stringstream ss;

            struct tm *local_time = localtime(&time);
            ss << local_time->tm_year + 1900 << "-" << local_time->tm_mon + 1
               << "-" << local_time->tm_mday << " " << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec;

            return ss.str();
        }

        static void listShow(const httplib::Request &req, httplib::Response &res)
        {
            // 先获取所有文件信息
            std::vector<cloud::BackupInfo> arr;
            data->getAll(&arr);

            // 再根据所有文件信息构建http响应
            std::stringstream ss;
            ss << "<!DOCTYPE html>";
            ss << "<html lang='ch'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            ss << "<title>Download</title></head>";
            ss << "<body><h1>Download</h1><table>";

            for (auto &s : arr)
            {
                ss << "<tr>";

                ss << "<td><a href='" << s._url_path << "'>" << util(s._real_path).fileName() << "</a></td>";

                ss << "<td align='right'>  " << "\t" << converter(s._mtime) << " </td>";

                ss << "<td align='right'>  " << "\t" << s._fsize / 1024 << "K</td>";

                ss << "</tr>";
            }

            ss << "</table></body></html>";

            res.body = ss.str();
            res.set_header("Content-Type", "text/html");
            res.status = 200;
        }

        static std::string getETag(const BackupInfo &info)
        {
            // 设置头部字段中etag字段，格式：filename-fsize-mtime

            util fu(info._real_path);
            std::string etag = fu.fileName();
            etag += "-";
            etag += std::to_string(info._fsize);
            etag += "-";
            etag += std::to_string(info._mtime);

            return etag;
        }

        static void download(const httplib::Request &req, httplib::Response &res)
        {
            // 根据请求获取文件信息
            BackupInfo info;
            data->getOneByURL(req.path, &info);
            // 判断该文件是否被压缩
            // 如果被压缩，先解压，删除压缩包，修改备份信息
            if (info._pack_flag)
            {
                util fu(info._pack_path);
                fu.uncompress(info._real_path); // 解压缩
                fu.rmdir();
                info._pack_flag = false;
                data->update(info);
            }
            // 将文件内容放入res.body中
            util fu(info._real_path);

            bool breakpoint_resume = false; // 判断是否是断点续传标识符
            std::string old_etag;
            if (req.has_header("If-Range"))
            {
                // 有If-Range字段说明是断点续传
                old_etag = req.get_header_value("If-Range");
                // 如果If-Range字段的值和请求文件的etag值一致，说明请求文件没有被修改过，设置为true，反之false
                if (old_etag == getETag(info))
                {
                    breakpoint_resume = true;
                }
            }
            if (!breakpoint_resume)
            {
                // 普通的文件下载请求
                fu.getContent(&res.body);

                //  设置响应头部字段
                res.set_header("Accept-Ranges", "bytes");
                res.set_header("ETag", getETag(info));
                res.set_header("Content-Type", "application/octet-stream");
                res.status = 200;
            }
            else
            {
                fu.getContent(&res.body);
                res.set_header("Accept-Ranges", "bytes");
                res.set_header("ETag", getETag(info));
                res.status = 206;
            }
        }

    private:
        int _server_port;
        std::string _server_ip;
        std::string _download_prefix;
        httplib::Server _server;
    };
}