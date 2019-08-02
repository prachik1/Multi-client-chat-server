#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h>  
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/sem.h>
#include <math.h>
#include <fcntl.h>

int socket_id;

void handle(int temp) 
{
	printf("\nClosing connections.\n");
	close(socket_id);
	exit(0);
}

int main()
{
	int x, n;					//file descriptor-stores value returned by socket system call
	int portno = 6111,cli_id;			//port no on which server accepts connections
	char buffer[255];
	off_t size;
	char temp[255];
	char temp1[255];
	char temp2[255];
	struct hostent *server;
	struct sockaddr_in server_address; 
	char *token = NULL;
    token = (char*)malloc(sizeof(char)*30);
    struct stat finfo;
    off_t fsize;
    FILE *fp,*fp1;

 	socket_id = socket(AF_INET, SOCK_STREAM, 0);		
 	bzero((char *) &server_address, sizeof(server_address));
 	server_address.sin_family = AF_INET; 						//Internet family of the server
 	server_address.sin_port = htons(portno);					//Same port number as the server
 	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");	//localhost address	

 	if (connect(socket_id,(struct sockaddr *) &server_address,sizeof(server_address)) < 0) 
 		{ 
 			printf("Error while connecting with the server.Check if server and client has same port number.\n"); 
 			return 0;
 		}
 	else
 		printf("Connected to server\n");

 	signal(SIGINT, handle);
 	signal(SIGQUIT, handle);
    signal(SIGTSTP, handle);

 	x = fork();
 	//child process writes to the server and parent reads from the server

 	//while(1)
 	//{
 		if(x == 0)
 		{
 			while(1)
 			{
				scanf("%[^\n]s",buffer);
				strcpy(temp,buffer);
				token = strtok(temp," ");
				if(strcmp(token,"/sendfile")!=0)
				{
	 				n = write(socket_id,buffer,255);
	 				if(n <= 0)
		    		{
						perror("Write Error: ");
						close(socket_id);
						break;
					}
					getchar();
				}
				else
				{
					token = strtok(NULL, " ");
					cli_id = atoi(token);
					token = strtok(NULL, " ");
					strcpy(temp,token);
					fp = fopen(temp, "r");

					stat(temp, &finfo);

    				fsize = (intmax_t)finfo.st_size;
    				memset(buffer,0,255);
    				sprintf(buffer,"/sendfile %d\n %jd\n",cli_id,fsize);
    				n = write(socket_id,buffer,255);

					memset(buffer,0,255);
					size = fsize;

					while (!feof(fp))
    				{
    					if(size > sizeof(buffer))
    					{
	        				fread(buffer, sizeof(buffer), 1, fp); 
	        				n = write(socket_id,buffer,255);
	        				size = size - sizeof(buffer);
	        				memset(buffer,0,255);
        				}
        				else
        				{
        					fread(buffer, fsize, 1, fp); 
	        				n = write(socket_id,buffer,255);
	        				memset(buffer,0,255);
        				}
        			}

        			fclose(fp);
					
				}
				//getchar();
 			}
 			exit(0);
 		}

 		else
 		{
 			//client could receive a broadcast, client close, other client, group message.
			while(1)
 			{
				bzero(buffer,255);
 				n = read(socket_id,buffer,255);
				if(n>0)
				{
 					printf("%s\n",buffer);
 					// sscanf(buffer,"%s:%s",temp1,temp2);
 					// printf("%s:%s",temp1,temp2);
 					// if(strcmp(temp2,"receive")==0)
 					// {
 					// 	n = read(socket_id,buffer,255);
 					// 	fp1 = fopen("tempo112.txt", "a+");
 					// 	fwrite(buffer, n, 1, fp);
 					// 	fclose(fp);
 					// }
				}
 				else if(n <= 0)
        		{
					perror("Read Error: ");
					close(socket_id);
					break;
				} 
				else if(n == 0)
				{
					perror("Read Error : ");
					close(socket_id);
					break;
				}
 			}
 			exit(0);
		}
 	//}

	return 0;
}
