#pragma once

#include "data.hpp"
#include "httplib.h"
#include <Windows.h>

namespace cloud
{
#define SERVER_IP "172.29.65.234"
#define SERVER_PORT 9000

#define BACKUP_FILE "./backup.dat"
#define BACK_DIR "./backup/"

	class Backup
	{
	public:
		Backup(const std::string &back_dir, const std::string &filename):_back_dir(back_dir)
		{
			_data = new DataManager(filename);
		}

		std::string getFileIdentifier(const std::string &filename)
		{
			util fu(filename);
			std::stringstream ss;
			ss << fu.fileName() << "-" << fu.fileSize() << "-" << fu.lastModTime();
			return ss.str();
		}


		bool upload(const std::string &filename)
		{
			util fu(filename);
			std::string body;
			fu.getContent(&body);

			httplib::Client client(SERVER_IP, SERVER_PORT);
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
			//�жϸ��ļ��Ƿ���Ҫ�ϴ����ж����ݣ�1���ļ������� 2���ļ����ڵ����޸Ĺ�
			//1���Ƿ��������ļ���ֻҪ�жϱ�����Ϣ��û�и��ļ���Ϣ����
			//2���Ƿ��޸�ֻ��Ҫƥ���ļ���ʶ���Ƿ�һ�¼���

			//����ֵ������ϴ��˷���true��û�ϴ�����false

			//���ж��費��Ҫ�ϴ�
			std::string file_id;
			if (_data->getOneByKey(filename, &file_id) != false)
			{
				//˵���ļ�����
				//�ж��ļ���ʶ���Ƿ�һ��
				std::string id = getFileIdentifier(filename);
				if (file_id == id)
				{
					//��ȣ�����Ҫ�ϴ�
					return false;
				}
			}

			//�ϴ��߼�
			//���ﻹ��һ���ط���Ҫע��,����ļ���Ƶ��д�룬��ô�ͻ�Ƶ���ϴ������������趨һ��ʱ�䣬�����趨30�룬30����û�б��޸Ĳ��ϴ�
			util fu(filename);
			if (time(nullptr) - fu.lastModTime() < 3) return false;

			return upload(filename);

		}

		bool runModule()
		{
			while (true)
			{
				//��ȡ�����ļ���Ϣ
				util fu(_back_dir);
				std::vector<std::string> arr;
				fu.scanDirectory(&arr);
				//�ϴ�
				for (auto& file : arr)
				{
					//����ϴ��ɹ�������������Ϣ������ϴ�ʧ�ܲ����κδ�����Ϊ������ѭ������һ�λ�������ϴ�
					if(uploader(file) == true)
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