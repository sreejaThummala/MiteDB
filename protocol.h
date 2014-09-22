#ifndef MITEDB_PROTOCOL_H
#define MITEDB_PROTOCOL_H
#include <string>
#include <bitset>
using namespace std;
typedef struct{
	bitset<2> cmd;
	bitset<8> key_len;
	bitset<16> value_old_len;
	bitset<16> value_new_len;
}RequestHeader;
typedef struct{
	string key;
	string value_old;
	string value_new;
}Request;
typedef struct{
	RequestHeader header;
	Request request;
}RequestMessage;
typedef struct{
	bitset<1> response;
	bitset<8> key_len;
	bitset<16> value_len;
}ResponseHeader;
typedef struct{
	string key;
	string value;
}Response;
typedef struct{
	ResponseHeader header;
	Response response;
}ResponseMessage;
#endif
