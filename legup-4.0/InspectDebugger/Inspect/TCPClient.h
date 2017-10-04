/* 
 * File:   TCPClient.h
 * Author: nazanin
 *
 * Created on June 16, 2013, 3:07 PM
 */

#ifndef TCPCLIENT_H
#define	TCPCLIENT_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

class TCPClient {
public:
    TCPClient(std::string hostname, std::string port);    
    virtual ~TCPClient();
    
#if defined(PYTHON_WRAPPER) || defined(DISCREP)
    bool openConnection();
#else
    void openConnection();
#endif
    void closeConnection();
    std::string sendMessage(std::string msg);    
    
    std::string hostname;
    std::string port;
    bool isConnected;
    
private:
        
    int sockfd, portNo, n;
    struct hostent* server;
    struct sockaddr_in serveraddr;

};

#endif	/* TCPCLIENT_H */

