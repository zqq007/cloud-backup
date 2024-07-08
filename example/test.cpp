#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <memory>
#include <jsoncpp/json/json.h>
#include "bundle.h"
#include "httplib.h"

// int main(int argc, char *argv[])
// {
//     // const char *name = "小明";
//     // int age = 18;
//     // int score[] = {78, 90, 87};

//     // Json::Value root;
//     // root["姓名"] = name;
//     // root["年龄"] = age;
//     // root["成绩"].append(score[0]);
//     // root["成绩"].append(score[1]);
//     // root["成绩"].append(score[2]);

//     // Json::StreamWriterBuilder builder;
//     // std::unique_ptr<Json::StreamWriter> swriter(builder.newStreamWriter());
//     // std::stringstream ss;
//     // swriter->write(root, &ss);

//     // std::cout << ss.str() << std::endl;

//     // std::string str = R"({"姓名":"小明","年龄":18,"成绩":[78, 90, 87]})";

//     // Json::Value val;
//     // Json::CharReaderBuilder r;
//     // std::unique_ptr<Json::CharReader> cr(r.newCharReader());
//     // std::string err;
//     // bool res = cr->parse(str.c_str(), str.c_str() + str.size(), &val, &err);
//     // if (!res)
//     // {
//     //     std::cerr << "parse err:" << err << std::endl;
//     //     return -1;
//     // }
//     // std::cout << val["姓名"].asString() << std::endl;
//     // std::cout << val["年龄"] << std::endl;
//     // std::cout << val["成绩"][0] << std::endl;
//     // std::cout << val["成绩"][1] << std::endl;
//     // std::cout << val["成绩"][2] << std::endl;

//     // std::string str = R"("xiaoming")";
//     // std::cout << str << std::endl;

//     if (argc != 3)
//     {
//         std::cerr << "underparameter" << std::endl;
//         return -1;
//     }

//     std::string ifilename = argv[1];
//     std::string ofilename = argv[2];

//     std::fstream ifs;
//     ifs.open(ifilename, std::ios::binary);
//     ifs.seekg(0, std::ios::end);
//     size_t fsize = ifs.tellg();
//     ifs.seekg(0, std::ios::beg);

//     std::string body;
//     body.resize(fsize);
//     ifs.read(&body[0], fsize);

//     std::string packed = bundle::pack(bundle::LZIP, body);

//     std::fstream ofs;
//     ofs.open(ofilename, std::ios::binary);
//     ofs.write(&packed[0], fsize);

//     ifs.close();
//     ofs.close();
//     return 0;
// }

// #include <iostream>
// #include <string>
// #include <fstream>
// #include "bundle.h"
// int main(int argc, char *argv[])
// {
// std::cout << "argv[1] 是原始文件路径名称\n";
// std::cout << "argv[2] 是压缩包名称\n";
//     if (argc < 3)
//         return -1;
//     std::string ifilename = argv[1];
//     std::string ofilename = argv[2];
//     std::ifstream ifs;
//     ifs.open(ifilename, std::ios::binary); // 打开原始文件
//     ifs.seekg(0, std::ios::end);           // 跳转读写位置到末尾
//     size_t fsize = ifs.tellg();            // 获取末尾偏移量--文件长度
//     ifs.seekg(0, std::ios::beg);           // 跳转到文件起始
//     std::string body;
//     body.resize(fsize);                                    // 调整body大小为文件大小
//     ifs.read(&body[0], fsize);                             // 读取文件所有数据到body找给你
//     std::string packed = bundle::pack(bundle::LZIP, body); // 以lzip格式压缩文件数据
//     std::ofstream ofs;
//     ofs.open(ofilename, std::ios::binary); // 打开压缩包文件
//     ofs.write(&packed[0], packed.size());  // 将压缩后的数据写入压缩包文件
//     ifs.close();
//     ofs.close();
//     return 0;
// }

// int main(int argc, char *argv[])
// {
//     if (argc != 3)
//     {
//         std::cerr << "argv[1] 是原始文件路径名称\n";
//         std::cerr << "argv[2] 是压缩包名称\n";
//         return -1;
//     }

//     std::string ifilename = argv[1]; // 压缩文件
//     std::string ofilename = argv[2]; // 解压缩之后的文件

//     std::ifstream ifs;
//     ifs.open(ifilename, std::ios::binary);
//     ifs.seekg(0, std::ios::end);
//     size_t fsize = ifs.tellg();
//     ifs.seekg(0, std::ios::beg);

//     std::string body;
//     body.resize(fsize);

//     ifs.read(&body[0], fsize);

//     std::string unpacked = bundle::unpack(body);

//     std::ofstream ofs;
//     ofs.open(ofilename, std::ios::binary);
//     ofs.write(&unpacked[0], unpacked.size());

//     ifs.close();
//     ofs.close();

//     return 0;
// }

void Hello(const httplib::Request &req, httplib::Response &rsq)
{
    rsq.set_content("hello world", "text/plain");
    rsq.status = 200;
}

void Number(const httplib::Request &req, httplib::Response &rsq)
{
    auto num = req.matches[1];
    rsq.set_content(num, "text/plain");
    rsq.status = 200;
}

void Mutipart(const httplib::Request &req, httplib::Response &rsq)
{
    auto ret = req.has_file("file");
    if (!ret)
    {
        std::cout << "The file could not be found";
        rsq.status = 404;
        return;
    }

    const auto &file = req.get_file_value("file");
    rsq.body.clear();
    rsq.body += file.filename;
    rsq.body += "\n";
    rsq.body += file.content;
    rsq.set_header("Content-Type", "text/plain");
    rsq.status = 200;
    return;
}

int main()
{
    httplib::Server server;
    server.Get("/hi", Hello);
    server.Get(R"(/numbers/(\d+))", Number);
    server.Post("/multipart", Mutipart);
    server.listen("0.0.0.0", 9090);
    return 0;
}