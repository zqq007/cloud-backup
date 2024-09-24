#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "data.hpp"
#include "httplib.h"
#include <Windows.h>

namespace cloud
{
#define SERVER_IP "101.34.67.147"
#define SERVER_PORT 9000

#define BACKUP_FILE "./backup.dat"
#define BACK_DIR "./backup/"

	class Backup
	{
	public:
		Backup(const std::string& back_dir, const std::string& filename) :_back_dir(back_dir)
		{
			_data = new DataManager(filename);
		}

		std::string getFileIdentifier(const std::string& filename)
		{
			util fu(filename);
			std::stringstream ss;
			ss << fu.fileName() << "-" << fu.fileSize() << "-" << fu.lastModTime();
			return ss.str();
		}


		bool upload(const std::string& filename)
		{
			util fu(filename);
			std::string body;
			fu.getContent(&body);

			httplib::SSLClient client(SERVER_IP, SERVER_PORT);

			// 禁用证书验证
			client.enable_server_certificate_verification(false);

			httplib::MultipartFormData item;
			item.content = body;
			item.content_type = "application/octet-stream";
			item.filename = fu.fileName();
			item.name = "file";

			httplib::MultipartFormDataItems items;
			items.push_back(item);

			auto res = client.Post("/upload", items);
			if (!res || res->status != 200) return false;
			return true;
		}

		bool uploader(const std::string& filename)
		{
			//判断该文件是否需要上传，判断依据：1、文件是新增 2、文件存在但被修改过
			//1、是否新增了文件：只要判断备份信息有没有该文件信息即可
			//2、是否被修改只需要匹对文件标识符是否一致即可

			//返回值：如果上传了返回true，没上传返回false

			//先判断需不需要上传
			std::string file_id;
			if (_data->getOneByKey(filename, &file_id) != false)
			{
				//说明文件存在
				//判断文件标识符是否一致
				std::string id = getFileIdentifier(filename);
				if (file_id == id)
				{
					//相等，不需要上传
					return false;
				}
			}

			//上传逻辑
			//这里还有一个地方需要注意,如果文件被频繁写入，那么就会频繁上传，所以我们设定一个时间，这里设定30秒，30秒内没有被修改才上传
			util fu(filename);
			if (time(nullptr) - fu.lastModTime() < 3) return false;

			return upload(filename);

		}

		bool runModule()
		{
			while (true)
			{
				//获取所有文件信息
				util fu(_back_dir);
				std::vector<std::string> arr;
				fu.scanDirectory(&arr);
				//上传
				for (auto& file : arr)
				{
					//如果上传成功才新增备份信息，如果上传失败不做任何处理，因为这是死循环，下一次还会继续上传
					if (uploader(file) == true)
						_data->insert(file, getFileIdentifier(file));
				}
				Sleep(1000);
			}
		}

	private:
		std::string _back_dir;
		DataManager* _data;
	};
}