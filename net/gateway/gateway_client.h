#pragma once

//void * gateway_server_start(unsigned short port);
//void *gateway_client_thread(void *arg);
void* gateway_client_thread(SOCKET_T sockfd);
