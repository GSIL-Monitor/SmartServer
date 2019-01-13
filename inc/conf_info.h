#pragma once
#include <fstream>
#include "log.h"

class FILE_RAII 
{
public:
	FILE_RAII( std::ifstream* ifs)
	{
		m_ptr_ifs = ifs;
	}
	~FILE_RAII()
	{
		m_ptr_ifs->close();
		//LogD("FILE_RAII close ifstream\n");
	}
private:
	std::ifstream* m_ptr_ifs;
};

int GetKeyValue(const std::string& filename, const std::string& section, const std::string& key, std::string& value);
