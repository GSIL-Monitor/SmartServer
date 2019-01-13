#pragma once
#include <atomic>
//extern unsigned int http_thread_num;
//extern std::mutex http_thread_num_mutex;
extern std::atomic<unsigned int> http_thread_num;

//void * socket_server(unsigned short port,const char *name,void *(*client_fun)(void* arg));
void * socket_server(unsigned short port,const char *name,void *(*client_fun)(SOCKET_T sockfd));
