#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define PORT 8888
#define BACKLOG 10
#define PRINT(msg) printf ("%s\n", msg)
#define USER "progjar"
#define PASS "utsprogjar"
#define ROOTDIR "FTP"

int readresponse(int, char *);

int main ()
{
	int sockserv, retval, clisize;

	int sockfd, sockcli, auth = 0;
	struct sockaddr_in servaddr, cliaddr;
	char buf[200], respbuf[200], user[200], pass[200];
	
	sockfd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	PRINT("Socket dibuat !");

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	retval = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	PRINT("Port Binding berhasil !");

	retval = listen(sockfd, BACKLOG);

	sockcli = accept(sockfd,(struct sockaddr*)&cliaddr, &clisize);
	PRINT("Accept !");
	sprintf(respbuf,"220 FTP server ready\r\n");
	retval = send(sockcli, respbuf, strlen(respbuf),0);
	while(retval != -1){
		retval = readresponse(sockcli, buf);
		if (retval == -1)
			break;
		else if (strncasecmp(buf, "USER",4) == 0){
			PRINT(buf);
			sscanf(buf,"%*s %s",user);
			if (strcmp(user,USER) == 0){
				sprintf(respbuf,"331 User name okay, need password.\r\n");
				retval = send(sockcli, respbuf, strlen(respbuf),0);
				retval = readresponse(sockcli, buf);
				PRINT(buf);
				sscanf(buf,"%*s %s",pass);
				if (strcmp(pass,PASS) == 0){
					sprintf(respbuf,"230 User logged in, proceed.\r\n");
					retval = send(sockcli, respbuf, strlen(respbuf),0);
					auth = 1;
				}
				else{
					sprintf(respbuf,"530 Not logged in.\r\n");
					retval = send(sockcli, respbuf, strlen(respbuf),0);
				}
			}
			else{
				sprintf(respbuf,"530 Not logged in.\r\n");
				retval = send(sockcli, respbuf, strlen(respbuf),0);
			}
		}
		else if (strcasecmp(buf, "LIST") == 0){
			if (auth == 1){
				PRINT(buf);
				DIR *rootdir = opendir(ROOTDIR);

				struct  dirent *direntry;
				char dirbuf[256];

				strcpy(respbuf, "LIST");
				strcat(respbuf, " OK\r\n\r\n");
				retval = write(sockcli, respbuf, strlen(respbuf));
				direntry = readdir(rootdir);
				while((direntry = readdir(rootdir))!=NULL)
				{
					if(strcmp(".",direntry->d_name) !=0 && strcmp("..", direntry->d_name))
					{
						sprintf(dirbuf, "%s\r\n", direntry->d_name);
						retval= send(sockcli, dirbuf, strlen(dirbuf),0);
					}
				}

				strcpy(dirbuf, ".\r\n");
				retval = send(sockcli, dirbuf, strlen(dirbuf),0);

				closedir(rootdir);
			}
			else{
				sprintf(respbuf,"332 Need account for login.\r\n");
				retval = send(sockcli, respbuf, strlen(respbuf),0);
			}
		}
		else if (strcasecmp(buf, "PUT") == 0){
			PRINT(buf);
		}
		else if (strcasecmp(buf, "QUIT") == 0){
			sprintf(respbuf,"Terminating program...\r\n");
			retval = send(sockcli, respbuf, strlen(respbuf),0);
			retval = -1;
		}
		else{
			PRINT("Bad Request");
			sprintf(respbuf,"503 Bad sequence of commands.\r\n");
			retval = send(sockcli, respbuf, strlen(respbuf),0);
		}
	}
}

int readresponse(int sockfd, char *buf){
	char tmp;
	int retval, index = 0;

	do{
		retval = recv(sockfd, &tmp,sizeof(char), 0);
		if (retval > 0){
			if (tmp == '\r'){
				recv(sockfd, &tmp,sizeof(char), 0);
				buf[index] = '\0';
				break;
			}
			else
				buf[index++] = tmp;
		}
	} while (retval > 0);
	if (retval == -1)
		return -1;
	else
		return 0;
}
