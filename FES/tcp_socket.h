#pragma once

#include <iostream>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>

const std::string host = "127.0.0.1";
const uint32_t port = 50400;

int connect_socket(const std::string& host, uint32_t port);