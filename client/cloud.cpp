#include "util.hpp"
#include "data.hpp"
#include "cloud.hpp"


int main()
{
    /*cloud::util fu("./");

    std::vector<std::string> arr;
    fu.scanDirectory(&arr);
    cloud::DataManager manager(BACKUP_FILE);
    for (auto& file : arr)
    {
        manager.insert(file, "abcd");
    }*/

    /*cloud::DataManager manager(BACKUP_FILE);
    std::string str;
    manager.getOneByKey("./cloud.cpp", &str);
    std::cout << str << std::endl;*/

    cloud::Backup backup(BACK_DIR, BACKUP_FILE);
    backup.runModule();

    return 0;
}