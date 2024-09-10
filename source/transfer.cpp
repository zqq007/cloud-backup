#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "util.hpp"
#include "httplib.h"
#include "data.hpp"

// extern cloud::DataManager *data;

int main()
{
    // httplib::SSLServer svr("transfer-cert.pem", "transfer-key.pem");
    httplib::SSLServer svr("server-cert.pem", "server-key.pem");
    // httplib::Server svr; // 创建 HTTP 服务器

    std::string file_content;

    svr.set_pre_routing_handler([](const httplib::Request &req, httplib::Response &res)
                                {
        // 允许所有源访问 (可以将 '*' 换成特定的域名)
        res.set_header("Access-Control-Allow-Origin", "*");

        // 允许的请求方法
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");

        // 允许的请求头
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        // 处理预检请求 (OPTIONS)
        if (req.method == "OPTIONS") {
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }

        return httplib::Server::HandlerResponse::Unhandled; });

    // 路由处理函数: 传输 JSON 数组
    svr.Get("/data", [](const httplib::Request &, httplib::Response &res)
            {
                cloud::util file("./cloud.dat");
                std::string content;
                file.getContent(&content);

                Json::Value value(Json::arrayValue);

                cloud::JsonUtil::unserialize(content, &value);
                cloud::JsonUtil::serialize(value, &content);

                // 设置响应内容类型为 JSON 并返回数据
                res.set_content(content, "application/json");
                // res.set_header("Access-Control-Allow-Origin", "*"); // 允许所有来源
            });

    svr.Get("/photo", [](const httplib::Request &, httplib::Response &res){
        cloud::util file("./wwwroot/img/oICbA2KpwFCADANAEmEDeWfmIASjglArADkkM9~tplv-dy-aweme-images_q75.jpeg");
                std::string content;
                file.getContent(&content);
                res.set_content(content, "img/jpeg");
    });

    svr.listen("0.0.0.0", 8080); // 启动服务器监听 8080 端口

    // svr.listen("0.0.0.0", 8080);
}
