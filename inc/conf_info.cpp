#include <stdio.h>
#include <fstream>
#include <string>
#include "conf_info.h"

int GetKeyValue(const std::string& filename, const std::string& section, const std::string& key, std::string& value)
{
	std::ifstream ifs(filename.c_str());
	if(!ifs.is_open())
		return -1;
	class FILE_RAII Raii(&ifs);
	std::string line;
	while(1)
	{
		line.clear();
		getline(ifs,line);
		if((ifs.rdstate() & std::ifstream::eofbit) != 0)
			return -2;
		if(line.empty())
			continue;
		std::string info = "["+section+"]";	
		if(line.find(info) == std::string::npos)
			continue;
		while(1)
		{
			line.clear();
			getline(ifs,line);
			if((ifs.rdstate() & std::ifstream::eofbit) != 0)
				return -3;
			if(line.empty())
				continue;
			if(line.find(key+"=") == std::string::npos)
				continue;
			size_t indexL = line.find('=');
			value = line.substr(indexL+1, line.size()-indexL-1);
			return value.size();
		}
	}
}
