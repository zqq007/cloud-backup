#include "util.hpp"
#include "config.hpp"
#include "data.hpp"
#include "hot.hpp"
#include "service.hpp"
#include <thread>

void fileutil_Test(const std::string &filename)
{
    // cloud::util ut(filename);
    // std::cout << ut.fileSize() << std::endl;
    // std::cout << ut.lastModTime() << std::endl;
    // std::cout << ut.lastAccTime() << std::endl;
    // std::cout << ut.fileName() << std::endl;

    // cloud::util ut(filename);
    // std::string body;
    // ut.getContent(&body);

    // cloud::util out_ut("./temp.txt");
    // out_ut.setContent(body);

    // std::string packname = filename + ".lz";
    // cloud::util pack_util(filename);
    // pack_util.compress(packname);

    // cloud::util unpack_util(packname);
    // unpack_util.uncompress("./temp.txt");

    cloud::util _util(filename);
    _util.createDirectory();
    std::vector<std::string> array;
    _util.scanDirectory(&array);
    for (auto &a : array)
    {
        std::cout << a << std::endl;
    }
}

void jsonUtil()
{
    Json::Value root;
    const char *name = "Tom";
    int age = 20;
    float score[] = {88.3, 100, 90};
    root["姓名"] = name;
    root["年龄"] = age;
    for (int i = 0; i < sizeof(score) / sizeof(score[0]); ++i)
    {
        root["成绩"].append(score[i]);
    }

    std::string json_str;
    cloud::JsonUtil::serialize(root, &json_str);

    std::cout << json_str << std::endl;

    Json::Value val;
    cloud::JsonUtil::unserialize(json_str, &val);

    std::cout << val["姓名"].asString() << std::endl;
    std::cout << val["年龄"].asInt() << std::endl;
    for (int i = 0; i < sizeof(score) / sizeof(score[0]); ++i)
    {
        std::cout << val["成绩"][i] << std::endl;
    }
}

void configTest()
{
    cloud::Config *config = cloud::Config::getInstance();
    std::cout << config->getHotTime() << std::endl;
    std::cout << config->getServerPort() << std::endl;
    std::cout << config->getServerIP() << std::endl;
    std::cout << config->getDownloadPrefix() << std::endl;
    std::cout << config->getPackFileSuffix() << std::endl;
    std::cout << config->getPackDir() << std::endl;
    std::cout << config->getBackDir() << std::endl;
    std::cout << config->getBackupFile() << std::endl;
}

void dataTest(const std::string &file)
{
    cloud::BackupInfo backinfo;
    cloud::DataManager manager;
    std::vector<cloud::BackupInfo> arr;
    manager.getAll(&arr);
    for (auto &temp : arr)
    {
        std::cout << temp._atime << std::endl;
        std::cout << temp._fsize << std::endl;
        std::cout << temp._mtime << std::endl;
        std::cout << temp._pack_flag << std::endl;
        std::cout << temp._pack_path << std::endl;
        std::cout << temp._real_path << std::endl;
        std::cout << temp._url_path << std::endl;
    }
    // cloud::BackupInfo backinfo;
    // backinfo.createBackupInfo(file);

    // cloud::DataManager manager;
    // manager.insert(backinfo);

    // cloud::BackupInfo temp;
    // manager.getOneByURL("./download/bundle.h", &temp);

    // // 查看结构体各个信息
    // std::cout << temp._atime << std::endl;
    // std::cout << temp._fsize << std::endl;
    // std::cout << temp._mtime << std::endl;
    // std::cout << temp._pack_flag << std::endl;
    // std::cout << temp._pack_path << std::endl;
    // std::cout << temp._real_path << std::endl;
    // std::cout << temp._url_path << std::endl;

    // std::cout << std::endl;

    // 测试修改、获取所有文件信息
    // backinfo._pack_flag = true;
    // manager.update(backinfo);

    // 获取所有文件信息
    // std::vector<cloud::BackupInfo> arr;
    // manager.getAll(&arr);
    // for (auto &it : arr)
    // {
    //     std::cout << it._atime << std::endl;
    //     std::cout << it._fsize << std::endl;
    //     std::cout << it._mtime << std::endl;
    //     std::cout << it._pack_flag << std::endl;
    //     std::cout << it._pack_path << std::endl;
    //     std::cout << it._real_path << std::endl;
    //     std::cout << it._url_path << std::endl;
    // }

    // std::cout << std::endl;

    // // 测试getOneByRealPath
    // manager.getOneByRealPath(file, &temp);
    // std::cout << temp._atime << std::endl;
    // std::cout << temp._fsize << std::endl;
    // std::cout << temp._mtime << std::endl;
    // std::cout << temp._pack_flag << std::endl;
    // std::cout << temp._pack_path << std::endl;
    // std::cout << temp._real_path << std::endl;
    // std::cout << temp._url_path << std::endl;

    // std::cout << std::endl;
}

cloud::DataManager *data;
void hotTest()
{
    cloud::HotManager hot_manager;
    hot_manager.runModule();
}

void serviceTest()
{
    cloud::Service srv;
    srv.runModule();
}

int main(int argc, char *argv[])
{
    data = new cloud::DataManager();

    // if (argc != 2)
    // {
    //     std::cerr << "argv[1] is file name" << std::endl;
    //     exit(-1);
    // }
    // fileutil_Test(argv[1]);
    // jsonUtil();
    // configTest();
    // dataTest(argv[1]);
    // hotTest();
    // serviceTest();

    std::thread thread_hot_manager(hotTest);
    std::thread thread_service(serviceTest);

    thread_hot_manager.join();
    thread_service.join();

    delete data;
    return 0;
}