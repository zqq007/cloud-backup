#pragma once

#include <iostream>
#include <unordered_map>
#include <sstream>
#include "util.hpp"

namespace cloud
{
	class DataManager
	{
	public:
		int split(std::string body, const std::string& seq, std::vector<std::string>* arr)
		{
			int count = 0;//����
			size_t pos = 0, index = 0;
			while (true)
			{
				pos = body.find(seq.c_str(), index);
				if (pos == std::string::npos) break;
				//��ֹ����ո�
				if (pos == index)
				{
					index += seq.size();
					continue;
				}

				arr->push_back(body.substr(index, pos - index));
				count++;
				index = pos + seq.size();
			}
			//���һ���ո����β��Ҫ�ǵü���
			if (index < body.size())
			{
				arr->push_back(body.substr(index));
				count++;
			}
			return count;
		}

		DataManager(const std::string& backup_file) :_backup_file(backup_file) 
		{
			//����ļ��������򴴽�
			util fu(_backup_file);
			if(!fu.exists())
			{
				std::ofstream ofs;
				ofs.open(_backup_file, std::ios::binary);
				if (ofs.is_open() == false)
				{
					std::cerr << "write open file failed" << std::endl;
					exit(-1);
				}
			}
			initLoad(); 
		}

		bool storage()
		{
			//��ȡ�����ļ���Ϣ,��������Ϣ�����ض���ʽ����֯
			std::stringstream ss;
			for (auto& it : _tables)
			{
				ss << it.first << " " << it.second << "\n";
			}

			util fu(_backup_file);
			fu.setContent(ss.str());

			return true;
		}

		bool initLoad()
		{
			util fu(_backup_file);
			std::string body;
			fu.getContent(&body);
			std::vector<std::string> arr;
			split(body, "\n", &arr);
			for (auto& s : arr)
			{
				std::vector<std::string> temp;
				split(s, " ", &temp);
				if (temp.size() != 2)
				{
					//˵�����ݸ�ʽ�������⣬ֱ��continue
					continue;
				}
				_tables[temp[0]] = temp[1];
			}
			return true;
		}

		bool insert(const std::string &key, const std::string &val)
		{
			_tables[key] = val;
			storage();
			return true;
		}

		bool update(const std::string &key, const std::string &val)
		{
			_tables[key] = val;
			storage();
			return true;
		}

		bool getOneByKey(const std::string &key, std::string *val)
		{
			auto it = _tables.find(key);
			if (it == _tables.end()) return false;
			*val = it->second;
			return true;
		}

	private:
		std::string _backup_file;
		std::unordered_map<std::string, std::string> _tables;//key���ļ�·�����ƣ� value���ļ�Ψһ��ʶ��
	};
}
