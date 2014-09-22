#ifndef MITEDB_LOGFILE_H
#define MITEDB_LOGFILE_H
#include <string>
#include <iostream>
#include <fstream>
#include "stdafx.h"
#include "logRecord.hpp"
class LogFile{
	char* file="map.log";
	public:
//		std::ifstream input;
//		std::ofstream output;
	
		LogFile();
		std::ostream& binary_write (std::ostream& stream,logRecord& value);
		std::istream& binary_read(std::istream* stream, logRecord& value);
		int set_pair(std::string key, std::string value);
		std::string get(std::string key);
	};
