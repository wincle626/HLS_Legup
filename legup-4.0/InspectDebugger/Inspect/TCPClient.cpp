/* 
 * File:   TCPClient.cpp
 * Author: nazanin
 * This class provides a TCP client that can connect to ModelSim and send/receive commands
 * Created on June 16, 2013, 3:07 PM
 */

#include "TCPClient.h"


TCPClient::TCPClient(std::string hostname, std::string port) {
    this->hostname = hostname;
    this->port = port;
    this->portNo = atoi(port.c_str());
    this->isConnected = false;
}

TCPClient::~TCPClient() {
}

#if defined(PYTHON_WRAPPER) || defined(DISCREP)
bool TCPClient::openConnection() {    
#else
void TCPClient::openConnection() {    
#endif
    std::cout << "opening connection" << std::endl;
    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        std::cout << "Error opening socket!\n";
    
    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname.c_str());
    if (server == NULL)
        std::cout << "No such host as " << hostname << std::endl;
        
    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portNo);
    
    /* connect: create a connection with the server */
    if (connect(sockfd, (sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        std::cout << "Error connecting!" << std::endl;  
#if defined(PYTHON_WRAPPER) || defined(DISCREP)
		return false;
#endif
    } else {
    	isConnected = true;
    	std::cout << "connection opened" << std::endl;                
	}
    
    char buf[1000];
    bzero(buf, 1000);
    //n = read(sockfd, buf, strlen(buf));
    n = recvfrom(sockfd,buf,1000,0,NULL,NULL);
    std::cout << "n: " << n << std::endl;
    std::string result(buf);
    std::cout << result << std::endl;

#if defined(PYTHON_WRAPPER) || defined(DISCREP)
	return true;
#endif
}

void TCPClient::closeConnection() {
    close(sockfd);
}

std::string TCPClient::sendMessage(std::string msg){    
    msg += "\n";
    //std::cout << "sending message: ";
    char buf[msg.size()];
    //bzero(buf, strlen(buf));
    bzero(buf, msg.size());
    memcpy(buf, msg.c_str(), msg.size());
    //fputs(buf, stdout);
    //std::cout << std::endl;
    n = write(sockfd, buf, msg.size());
    //n = sendto(sockfd,buf,strlen(buf),0, 
    //            (struct sockaddr *)&serveraddr,sizeof(serveraddr));
    /*if (n < 0)
        std::cout << "error writing to socket!" << std::endl;
    else
        std::cout << "message sent successfully!" << std::endl;*/
        
    
    //bzero(buf, strlen(buf));
    bzero(buf, msg.size());
    //n = read(sockfd, buf, strlen(buf));
    n = recvfrom(sockfd,buf,msg.size(),0,NULL,NULL);
    //std::cout << "n: " << n << std::endl;
    std::string result(buf);
    
    /*if (n < 0)
        std::cout << "error reading from socket!" << std::endl;    
    std::cout << "message received from server: " << result << std::endl;*/
    return result;
}

