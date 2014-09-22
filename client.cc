#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include "protocol.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

int main()
{  printf("hell");
   fflush(stdout);
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
	
   printf("hello");
   fflush(stdout);
   sockfd=socket(AF_INET,SOCK_STREAM,0);
   string address = "127.0.0.1";
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(address.c_str());
   servaddr.sin_port=htons(32000);
printf("connection");
fflush(stdout);
   connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
  printf("connection created"); 
  fflush(stdout);
   RequestMessage message;
   char key[] ={'1','2','3','4'};
   char value[] = {'o','r'};
   int key_len =4;
   int value_len = 2;
   bitset<2> cmd;
   cmd.set(1,1);
   cmd.set(0,1);
   message.header.cmd = cmd;
   message.header.key_len = key_len;
   message.header.value_new_len = value_len;
   message.header.value_old_len=0;
   
   char request[sizeof(message.header)+4+2];
   std::memcpy(request,&message.header,sizeof(message.header));
   std::memcpy(request,key,4);
   std::memcpy(request,value,2);
   printf("request created");
   fflush(stdout);
     int n1= sendto(sockfd,request,(sizeof(message.header)+6),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
	printf("%d",n1);
fflush(stdout);    
      printf("sent packet");
      fflush(stdout);
   ResponseHeader header;
   n=recvfrom(sockfd,&header,sizeof(ResponseHeader),0,NULL,NULL);
   int resp_size = (int)header.value_len.to_ulong()+(int)header.key_len.to_ulong();
   char response[resp_size];
   n=recvfrom(sockfd,response,resp_size,0,NULL,NULL);
   printf("%d",resp_size);
   fflush(stdout);
}


