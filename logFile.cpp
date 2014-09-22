#include "logFile.h"
#include "logRecord.hpp"

using namespace std;

std::ostream& binary_write(std::ostream& stream, const logRecord& value){
	    return stream.write(reinterpret_cast<const char*>(&value), sizeof(logRecord));
}

std::istream & binary_read(std::istream& stream, logRecord& value){
	    return stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}


LogFile::LogFile(){
	//output.open(file);
	//input.open(file);
}
