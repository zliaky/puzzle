#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

#define MYPORT 8888
#define BUFFER_SIZE 1024

char recvBuffer[1024];

int main(int argc, char* argv[])
{
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(MYPORT);
	serverAddr.sin_addr.s_addr = inet_addr("192.168.1.102");

	if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		cout << "connect error" << endl;
		perror("connect");
		exit(1);
	}

	char recvBuffer[BUFFER_SIZE];
	while (true)
	{
		send(clientSocket, "true", 5, 0);
		recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		cout << "receive message: " << recvBuffer << endl;
	}
	close(clientSocket);
	return 0;
}
