
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstring>
#include "protocol.h"
#include <vector>
#include "wqueue.h"
#include "thread.h"
#include "cache.hpp"
#include <iostream>
#include <fstream>
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_SYSTEM_NO_DEPRECATED
#include <boost/system/error_code.hpp>
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <boost/btree/btree_index_map.hpp>
#include <boost/btree/support/string_view.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace fs = ::boost::filesystem;
using namespace std;
using namespace boost::btree;
typedef btree_index_map<boost::string_view, boost::string_view> BT;
BT bt("st_index_map.l",flags::truncate);
vector<string> v;

const int CACHE_CAPACITY = 200;
cache<string,string>  lru_cache(CACHE_CAPACITY);
wqueue<WorkItem*> queue;
int size_log = 2048;
string current_log;
static int log_count = 0;
static int record_num=0;

typedef struct{
	string key;
	string value;
} Record;

std::string get_timestamp(){
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
	std::string t = boost::posix_time::to_iso_extended_string( now );
	return t;}
Record read(std::istream &is) {
	Record record;
	std::string line;
	std::getline(is,line);
	string delim=":";
	int cut = line.find_first_of(delim);
	string key = line.substr(0,cut);
	string value = line.substr(cut+1);
	record.key = key;
	record.value = value;
	return record;
}

void write(std::ostream &os, Record const &r) { 
	    os << &r.key;
	    os <<":";
	    os <<&r.value;
	    os<<"\n";
}

class Indexer: public Thread{
	wqueue<WorkItem*>& m_queue;
	public:
	Indexer(wqueue<WorkItem*>& queue): m_queue(queue){}
	void* run(){
	 //remove the logfile from the clue insert it into btree
	for(int i=0;;i++){
	WorkItem* item = m_queue.remove();
	 string file = item->getFile();
	 std::ifstream ifile;
	 ifile.open(file.c_str());
	    while(ifile){
		   Record record = read(ifile);
		   string key = record.key;
		   string value = record.value;
	           bt.emplace(key,value);
	    }
	   ifile.close();
	  // fs::remove(fs::path(file.c_str()));
	  int ret_code = std::remove(file.c_str());
	  if(ret_code == 0){
		  printf("file was removed");
	  }else {
		  printf("file was not removed");
	  }
	}
	}
	};
ResponseMessage success_response(string key, string value){
	ResponseMessage message;
	bitset<1> response_code;
	response_code.set(1,1);
	message.header.response = response_code;
	message.header.value_len = bitset<16>(value.length());
	message.header.key_len = bitset<8>(key.length());

	char value_array[value.length()];
	char key_array[key.length()];
	memcpy(key_array,key.c_str(),key.length());
	memcpy(value_array,value.c_str(),value.length());
	return message;
}

ResponseMessage get_error_response(){
	ResponseMessage message;
        bitset<1> response_code ;
	response_code.set(1,0);
	message.header.response = response_code;
	message.header.key_len =  bitset<8>(0);
	message.header.value_len = bitset<16>(0);
	return message;
}

ResponseMessage handle_get_request(RequestHeader header,Request request){
	int key_len = (int)header.key_len.to_ulong();
	ResponseMessage response;
	if(key_len<request.key.length()){
		response = get_error_response();
	}
	//check the cache
	if(lru_cache.exists(request.key)){
	string value = lru_cache.get(request.key);
	return success_response(request.key,value);}
	//check the logfile
	Record record;
	vector<string>::iterator it = v.begin();
	while(it!=v.end()) {
	    /* std::cout << someVector[i]; ... */
	    //search each file for the  record	
	    ifstream ifline((*it));
	    if(ifline){
	    while(ifline.good()){
	//	ifline>>record;
		 Record record;
		 std::string line;
		 std::getline(ifline,line);  
		 string delim=":";
		 int cut = line.find_first_of(delim);
		 string key = line.substr(0,cut);
		 string value = line.substr(cut+1);
		 record.key = key;
		 record.value = value;

		if(request.key==record.key){
			//construct response message and leave 
			return success_response(record.key,record.value);
		}
	    }
	    it++;
	    }else{
			it=v.erase(it);
	    }
	         	
	}
	
	btree_index_map<boost::string_view,boost::string_view>::const_iterator itr;
	boost::string_view k;
	k=request.key;
        itr=bt.find(k);
	if(itr==bt.end()){
		return get_error_response();
	}
	int i=0;
	std::pair<boost::string_view,boost::string_view> pair = *itr;
	boost::string_view value = pair.second;
	boost::string_view key = pair.first;
	if(key!=k){
		return get_error_response();
	}
	
	return success_response(key.to_string(),value.to_string());
	
	//check the btree

}
ResponseMessage handle_set_request(RequestHeader header,Request request){
	int key_len = (int)header.key_len.to_ulong();
	int value_new_len = (int) header.value_new_len.to_ulong();
	int value_old_len = (int) header.value_old_len.to_ulong();
	
	if(log_count>size_log){
		queue.add(new WorkItem(current_log));
	        current_log = "map"+get_timestamp()+".log";
		
	}
	ofstream ofline(current_log.c_str());
	ofline<<request.key;
	ofline<<":";
	ofline<<request.value_new;
	ofline<<"\n";
	log_count++;
	ofline.close();
		//add the current log to work item to be processed
		//create new log and add the 
	
}
ResponseMessage request_handler(RequestHeader header,Request request){
         bitset<2> cmd = header.cmd;
	 ResponseMessage response;
	 int cmd_int = (int)cmd.to_ulong();
	 if(cmd_int<0||cmd_int>2){
		 response = get_error_response();
	 }								                if(cmd_int==1){               
		 response = handle_get_request(header,request);
	 }
	 if(cmd_int==2){
		 response = handle_set_request(header,request);
	 }
}

void get_all(const fs::path& root,  string ext,vector<fs::path>& ret){
	if(!fs::exists(root)) return;
	if(fs::is_directory(root)){
		fs::recursive_directory_iterator it(root);
		fs::recursive_directory_iterator endit;
		 while(it != endit){
			 if (fs::is_regular_file(*it) and it->path().extension() == ext) {
				 ret.push_back(it->path().filename());
			 }
			 ++it;
		 }
	}
}

int main(int argc, char**argv)
{
   WorkItem* item;
   int listenfd,connfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t clilen;
   pid_t     childpid;
   RequestHeader header;
   current_log = "map"+get_timestamp()+".log";
   listenfd=socket(AF_INET,SOCK_STREAM,0);
   vector<fs::path> ret;
   const fs::path root(".");
   int workers= 10;
   printf("searching for log files");
   fflush(stdout);
   std::string ext = ".log";
   get_all(root,ext,ret);
   
	for(std::vector<fs::path>::size_type i = 0; i != ret.size(); i++){
		//add remaining stuff to queue
  	WorkItem* item = new WorkItem(ret[i].filename().c_str());
	queue.add(item);
	   //add to the wqueue and the other stuff
   }
   printf("search completed");
   fflush(stdout);

printf("start creating indexer threads");
fflush(stdout);
  //create an another thread for indexer and accept the crap
   for (int i = 0; i < workers; i++) {
	     Indexer* handler = new Indexer(queue);
		           if (!handler) {
				   printf("Could not create ConnectionHandler %d\n");
				   exit(1);
			   } 
			   handler->start();     
   }
printf("indexer threads are created");
fflush(stdout);
printf("start initing the server");
fflush(stdout);
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   listen(listenfd,1024);
printf("completed init the server");
fflush(stdout);
   for(;;)
   {
      clilen=sizeof(cliaddr);
      connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
      printf("%d",connfd);
     printf("accepted connection");
     fflush(stdout);

         for(;;)
         {
            //initially send a header length
            n = recvfrom(connfd,&header,sizeof(RequestHeader),0,(struct sockaddr *)&cliaddr,&clilen);
            int mesg_size = header.key_len.to_ulong()+header.value_old_len.to_ulong()+header.value_new_len.to_ulong(); 
	    //second send the original message request
	   char buffer[mesg_size];
	   n = recvfrom(connfd,buffer,mesg_size,0,(struct sockaddr *)&cliaddr,&clilen);

           Request request;
	   int key_len = (int)header.key_len.to_ulong();
	   int value_old_len = (int)header.value_old_len.to_ulong();
	   int value_new_len = (int)header.value_new_len.to_ulong();
	   char key[key_len];
	   char value_old[value_old_len];
	   char value_new[value_new_len];
	   std::memcpy((void*)key,(const_cast<char*>(buffer)),(int)header.key_len.to_ulong());
	   std::memcpy((void*)value_old,(const_cast<char*>(buffer)),(int)header.value_old_len.to_ulong());
	   std::memcpy((void*)value_new,(const_cast<char*>(buffer)),(int)header.value_new_len.to_ulong());
	   
	   request.key=string(key);
	   request.value_new = string(value_new);
	   request.value_old = string(value_old);

	   ResponseMessage message = request_handler(header,request);
	   ResponseHeader header = message.header;
	   sendto(connfd, &header,sizeof(header),0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
	   int resp_size = (int)header.key_len.to_ulong()+(int)header.value_len.to_ulong();
	  Response response = message.response;
	   sendto(connfd,&response,resp_size,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
            
         }
         
      
      close(connfd);
   }
}
