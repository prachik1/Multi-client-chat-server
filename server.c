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
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int newsocket_id;

void handle(int temp) {
    printf("\nClosing connections.\n");
    close(newsocket_id);
    exit(0);
}

typedef struct client_info
{
	int id[10];
	int sock[10];
	int NoOfGroups[10];
	int group_id[10][10];
	int permission[10][10];

}client_info;

typedef struct group_info
{
	int gid[10];
	int admin[10];
	int group_members[10][10];

}group_info;

typedef struct mesg_info
{
	int source[10];
	int dest[10];
	char message[10][255];

}mesg_info;


int main()
{
	int portno = 6111, id, i, n, j, temp, *count, id1, id2, id3, k, m, status, error, yes = 1, z, p = 0, cli_id, groupJoin, gr_id, flag = 0, flag1 = 0;
	int socket_id, semid, x, semid1, semid2, semid3, tid, size, opts;
	struct sockaddr_in server_address;  
    struct sockaddr_in client_address;
    struct sembuf pop, vop ;
    socklen_t length;
    char str[50]={0};
    char buffer[255];
	int groupMembers[10];
	int countMembers = 0;

	pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1 ; vop.sem_op = 1;
    semid = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);				//binary semaphore - client info table
    semctl(semid, 0, SETVAL, 1);
    semid1 = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);			//binary semaphore - group info table
    semctl(semid1, 0, SETVAL, 1);
    semid2 = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);			//binary semaphore - message info table
    semctl(semid2, 0, SETVAL, 1);
    semid3 = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);			//binary semaphore - count of clients
    semctl(semid3, 0, SETVAL, 1);

    bzero((char *) &server_address, sizeof(server_address)); //initialize server_address to zero
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost address 
    server_address.sin_port = htons(portno);        //converts a port number in host byte order to port number in network byte order
    socket_id = socket(AF_INET, SOCK_STREAM, 0);

    client_info *client = NULL;
    group_info *group = NULL;
    mesg_info *mesg = NULL;
    char *token = NULL;
    token = (char*)malloc(sizeof(char)*30);
    //client = (client_info*)malloc(sizeof(client_info));

    id = shmget(IPC_PRIVATE, sizeof(client_info), 0777 | IPC_CREAT);
   	client = (client_info*)shmat(id, 0, 0);
   	id1 = shmget(IPC_PRIVATE, sizeof(group_info), 0777 | IPC_CREAT);
   	group = (group_info*)shmat(id1, 0, 0);
   	id2 = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
   	count = (int*)shmat(id2, 0, 0);
   	id3 = shmget(IPC_PRIVATE, sizeof(mesg_info), 0777 | IPC_CREAT);
   	mesg = (mesg_info*)shmat(id3, 0, 0);

    if(socket_id < 0)
    {
        printf("Could not create socket");
        return 0;
    }

    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(0);
    }
        
    printf("\nSocket created\n");

    if(bind(socket_id, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 ) //binds a socket to an address
    {
        printf("\nError while binding.Initialize a different port number in main.\n");
        return 0; 
    }
        
    printf("\nBind successful\n");

    error = listen(socket_id,5);                   //server is listening on the socket for connections

    if(error == -1){
        perror("Listening : ");
        exit(0);
    }

    P(semid);
    P(semid1);
    P(semid2);

   	for (i=0; i<10; ++i)
   	{ 
   		client->id[i] = -1;				//stores the unique id of clients
   		client->NoOfGroups[i] = 0;		//stores the no Of groups for each client
   		group->gid[i] = -1;				//stores the unique id of groups
   		mesg->source[i] = -1;
   		mesg->dest[i] = -1;
   	}

   	for (i=0; i<10; ++i)
   	{ 
   		for (j=0; j<10; ++j)
   		{ 
   			client->group_id[i][j] = -1;		//group ids for each respective clients
   			group->group_members[i][j] = -1;	//client ids for each respective groups
   			client->group_id[i][j] = -1;
   			client->permission[i][j] = -1;
   		}
   	}

   	V(semid);
    V(semid1);
    V(semid2);
   	
   	P(semid3);
   	*count = 0;					//stores the count of active clients
   	V(semid3);

    while(1)
    {
   		length = sizeof(client_address);

    	newsocket_id = accept(socket_id, (struct sockaddr *) &client_address, &length);
    	status = fcntl(newsocket_id, F_SETFL, fcntl(newsocket_id, F_GETFL, 0) | O_NONBLOCK); //client side par how will become non-blocking?

    	if (status == -1)
    	{
  			perror("Error calling fcntl");
		}

		P(semid3);

		if(*count > 5)
   		{
   			n = write(newsocket_id,"Connection Limit Exceeded!",255);			// client shldn't connect so we will write on server socket id?
   		}

   		V(semid3);

    	x = fork();

    	if(x == 0)
    	{    		
	    	srand(time(NULL));
	    	P(semid3);
	    	(*count)++;					//no of active clients
	    	V(semid3);

	    	P(semid);

	    	for(i = 0; i < 5; i++)
	    	{
	    		if(client->id[i] == -1)			//This client is recognised by this i
	    			break;
		    }

	    	if(i != 5)
	    	{
	    		client->id[i] = 100000 + rand() % 90000;
	    		client->sock[i] = newsocket_id;
	    		client->NoOfGroups[i] = 0;
	    	}

	    	V(semid);

	    	sprintf(buffer,"Welcome to our chat service application! Your unique id is:%d\n",client->id[i]);
	    	n = write(newsocket_id,buffer,255);

	    	while(1)
	    	{
	    		signal(SIGINT, handle);             //handle function will return here making flag=0. Hence we need to break here. 
                signal(SIGQUIT, handle);
                signal(SIGTSTP, handle);

	    		bzero(buffer,255);
		    	n = read(newsocket_id,buffer,255);
		    	if(strcmp(buffer," ") == 0)      
                {
                    n = write(newsocket_id,"Enter a valid command",255);
                    continue;
                }

		    	else if(n > 0)
		    	{
		    		token = strtok(buffer," ");

			    	if(strcmp(token,"/active") == 0)
			    	{
			    		P(semid);
			    		for(j = 0; j < 5; j++)
			    		{
			    			if(client->id[j] != -1)
			    			{
			    				if(client->id[j] == client->id[i])
			    				{
			    					sprintf(buffer,"Client with id:%d is online. This is you!",client->id[j]);
			    					n = write(newsocket_id,buffer,255);
			    				}
			    				else
			    				{
			    				sprintf(buffer,"Client with id:%d is online",client->id[j]);
			    				n = write(newsocket_id,buffer,255);
			    				}
			    			}
			    		}
			    		V(semid);
			    	}

			    	else if(strcmp(token,"EXIT") == 0 || strcmp(buffer, "/quit") ==0)	
			    	{
			    		j = 0;
			    		
			    		sprintf(buffer,"Client with id %d is going offline.",client->id[i]);
			    		printf("Client with id %d is going offline.\n",client->id[i]);

			    		//all clients notified about this client termination

			    		P(semid);
			    		P(semid2);

			    		for(k = 0; k < 5; k++)
			    		{
			    			if(client->id[k] == -1 || client->id[k] == client->id[i])
			    			{
			    				continue;
			    			}
			    			else
			    			{
			    				for(j = 0; j < 10; j++)
			    				{
			    					if(mesg->source[j] == -1)
			    					{
			    						mesg->source[j] = client->id[i];
			    						mesg->dest[j] = client->id[k];
			    						strcpy(mesg->message[j],buffer);
			    						break;
			    					}
			    				}
			    			}
			    		}

			    		//remove all data entries from the message table

			    		for(k = 0; k < 10; k++)
			    		{
			    			if(mesg->source[k] == client->id[i] || mesg->dest[k] == client->id[i])
			    			{
			    				mesg->source[k] = -1;
			    				mesg->dest[k] = -1;
			    				strcpy(mesg->message[k]," ");
			    			}
			    			else
			    				continue;
			    		}

			    		V(semid2);
			    		P(semid1);

			    		//remove client from all its groups 

			    		for(k = 0; k < 5; k++)
			    		{
			    			j = 0;
			    			if(client->id[k] == client->id[i])				//remove this client
			    			{
			    				while(client->NoOfGroups[i] > 0)			//for all groups in which client is
			    				{
			    					if(client->group_id[i][j] > 0)
			    					{	
			    						temp = client->group_id[i][j];

			    						if(group->admin[temp-1] != client->id[i])	//client is not the admin of this group
				    					{
				    						for(p = 0; p < 10; p++)
				    						{
				    							if(group->group_members[temp-1][p] == client->id[i])	//remove client from group
				    							{
				    								group->group_members[temp-1][p] = -1;
				    								break;
				    							}
				    						}
				    					}

				    					else
				    					{
				    						group->gid[temp-1] = -1;
				    						group->admin[temp-1] = -1;

				    						for(m = 0 ; m < 5; m++)
				    						{
				    							if(client->id[m] != -1)
				    							{
				    								for(p = 0; p < 10; p++)
				    								{
				    									if(client->group_id[m][p] == temp)
				    									{
				    										client->group_id[m][p] = -1;		//delete the group
				    										client->NoOfGroups[m] -= 1;
				    									}
				    								}
				    							}
				    						}
				    					}
				    					client->NoOfGroups[i] -= 1;
			    					}
			    					j++;
			    				}
			    			}
			    		}
			    		V(semid1);

			    		//delete all client info
			    		client->id[i] = -1;
			    		client->NoOfGroups[i] = 0;

			    		V(semid);

			    		close(newsocket_id);
			    		exit(0);
			    	}

			    	else if(strcmp(token,"/broadcast") == 0)
			    	{
			    		j = 0;
			    		bzero(str,50);
			    		token = strtok(NULL, " ");
			    		while(token != NULL)
			    		{
			    			strcat(str,token);
			    			strcat(str," ");
				    		token = strtok(NULL, " ");
				    	}
				    	sprintf(buffer,"%s",str);

						if(*buffer != 0)
						{
							P(semid);
							P(semid2);

					    	for(k = 0; k < 5; k++)
				    		{
				    			if(client->id[k] == -1 || client->id[k] == client->id[i]) //excluding the client sending broadcast to all
				    			{
				    				continue;
				    			}
				    			else
				    			{
				    				for(j = 0; j < 10; j++)
				    				{
				    					if(mesg->source[j] == -1)
				    					{
				    						mesg->source[j] = client->id[i];
				    						mesg->dest[j] = client->id[k];
				    						strcpy(mesg->message[j],buffer);
				    						break;
				    					}
				    				}
				    			}
				    		}

				    		V(semid);
				    		V(semid2);
						}

						else
						{
							n = write(newsocket_id,"Type a message.",255);
						}
			    		
			    	}

			    	else if(strcmp(token,"/send") == 0)
			    	{
			    		bzero(str,50);
			    		flag = 0;
			    		token = strtok(NULL, " ");

			    		if(token != NULL)
			    		{
			    			cli_id = atoi(token);

				    		if(cli_id > 0)
				    		{
				    			for(k = 0; k < 5; k++)
				    			{
				    				P(semid);
				    				if(cli_id == client->id[k])
				    				{
				    					flag = 1;
				    					V(semid);
				    					break;
				    				}
				    				V(semid);
				    			}
				    			if(flag == 0)
				    			{
				    				n = write(newsocket_id,"Invalid client.",255);
				    				continue;
				    			}	
								token = strtok(NULL, " ");
						    	while(token != NULL)
						    	{
						    		strcat(str,token);
						    		strcat(str," ");
							    	token = strtok(NULL, " ");
							    }
							    sprintf(buffer,"%s",str);

								if(*buffer != 0)
				    			{
									P(semid2);

									for(j = 0; j < 10; j++)
									{
										if(mesg->source[j] == -1)
										{
											mesg->source[j] = client->id[i];
						    				mesg->dest[j] = cli_id;
						    				strcpy(mesg->message[j],buffer);
						    				break;
										}
									}

									V(semid2);
								}

								else
								{
									n = write(newsocket_id,"Type a message.",255);
								}
							}

							else
							{
								n = write(newsocket_id,"Invalid command",255);
							}
						}
						else
							n = write(newsocket_id,"To whom should I send?",255);				    						    		
			    	}

			    	else if(strcmp(token,"/sendgroup") == 0)
			    	{
			    		bzero(str,50);
			    		token = strtok(NULL, " ");

			    		if(token != NULL)
			    		{	
				    		gr_id = atoi(token);
				    		flag = 0;

				    		if(gr_id > 0)
							{
								for(k = 0; k < 5; k++)
				    			{
				    				P(semid1);
				    				if(gr_id == group->gid[k])
				    				{
				    					flag = 1;
				    					V(semid1);
				    					break;
				    				}
				    				V(semid1);
				    			}
				    			if(flag == 0)
				    			{
				    				n = write(newsocket_id,"Invalid Group.",255);
				    				continue;
				    			}
								token = strtok(NULL, " ");
					    		while(token != NULL)
					    		{
					    			strcat(str,token);
					    			strcat(str," ");
						    		token = strtok(NULL, " ");
						    	}
						    	sprintf(buffer,"%s",str);

						    	if(*buffer != 0)
								{
									P(semid1);
									P(semid2);
									
									for(j = 0; j < 10; j++)
									{
										if(mesg->source[j] == -1)
										{
											mesg->source[j] = client->id[i];
							    			mesg->dest[j] = group->admin[gr_id-1];
							    			strcpy(mesg->message[j],buffer);
							    			break;
										}
									}
									
									for(k = 0; k < 10; k++)
									{
										if(group->group_members[gr_id-1][k] != -1)
										{
											for(j = 0; j < 10; j++)
											{
												if(mesg->source[j] == -1)
												{
													mesg->source[j] = client->id[i];
							    					mesg->dest[j] = group->group_members[gr_id-1][k];
							    					strcpy(mesg->message[j],buffer);
							    					break;
												}
											}

										}
									}

									V(semid1);
									V(semid2);
								}

								else
								{
									n = write(newsocket_id,"Type a message.",255);
								}
							}

							else
							{
								n = write(newsocket_id,"Invalid command",255);
							}
						}
						else
							n = write(newsocket_id,"To whom should I send?",255);			    						    		
			    	}

			    	else if(strcmp(token,"/makegroup") == 0)						//with tokenization there shld be space in between. 
			    	{
			    		k = 0;
			    		j = 0;
			    		flag = 0;
			    		flag1 = 0;
			    		bzero(groupMembers,10);
			    		countMembers = 0;
			    		token = strtok(NULL, " ");
			    		if(token != NULL)
						{
				    		while(token != NULL)
				    		{
				    			flag = 0;
				    			flag1 = 0;
				    			countMembers++;
				    			groupMembers[j] = atoi(token);		//store clients forming a group
				    			P(semid);
				    			for(k = 0; k < 5; k++)
				    			{
				    				if(groupMembers[j] == client->id[k] && groupMembers[j] != client->id[i])
				    				{
				    					flag = 1;
				    					break;
				    				}
				    			}
				    			V(semid);
				    			if(flag == 0)
				    			{
				    				flag1 = 1;
				    				n = write(newsocket_id,"Invalid Client.Are you including yourself or someone who doesn't exist?.",255);
				    				break;
				    			}
				    			token = strtok(NULL, " ");
				    			j++;
				    		}

				    		if(flag1 == 1)
				    			continue;

				    		if(countMembers > 9)
				    		{
				    			n = write(newsocket_id,"Too many members. More than 10 not allowed",255);
				    			continue;
				    		}
				    		//update noOfGroups and groupId for each client. Create a group with all the information and state the admin.

				    		P(semid1);

				    		for(k = 0; k < 10; k++)
		    				{
		    					if(group->gid[k] == -1)
		    						break;
		    				}

		    				group->gid[k] = k+1;					//store the id
		    				group->admin[k] = client->id[i];		//admin info

		    				for(j = 0; j < countMembers; j++)
		    				{
		    					group->group_members[k][j] = groupMembers[j];	//populate the group with its members
		    				}
							
							V(semid1);

							P(semid);

							client->NoOfGroups[i] += 1;
							for(m = 0; m < 10; m++)
				    		{
				    			if(client->group_id[i][m] == -1)
								{
				    				client->group_id[i][m] = group->gid[k];
									break;
								}
				    		}

		    				//update for the client
				    		for(j = 0; j < countMembers; j++)
				    		{
				    			cli_id = groupMembers[j];

				    			for(p = 0; p < 5; p++)
				    			{
				    				if(client->id[p] == cli_id)
									{
				    					break;
									}
				    			}

				    			client->NoOfGroups[p] += 1;

				    			for(m = 0; m < 10; m++)
				    			{
				    				if(client->group_id[p][m] == -1)
									{
				    					client->group_id[p][m] = group->gid[k];
										break;
									}
				    			}
				    		}

				    		V(semid);
						}

						else
							n = write(newsocket_id,"Don't want any members?",255);

						n = write(newsocket_id,"Group Created",255);
			    	}

			    	else if(strcmp(token,"/makegroupreq") == 0)					
			    	{
			    		//bzero(buffer,255);
			    		bzero(groupMembers,10);
			    		countMembers = 0;
			    		flag = 0;
			    		flag1 = 0;
			    		k = 0;
			    		j = 0;
			    		token = strtok(NULL, " ");
			    		if(token != NULL)
			    		{
				    		while(token != NULL)
				    		{
				    			flag = 0;
				    			flag1 = 0;
				    			countMembers++;
				    			groupMembers[j] = atoi(token);		//store clients forming a group
				    			P(semid);
				    			for(k = 0; k < 5; k++)
				    			{
				    				if(groupMembers[j] == client->id[k] && groupMembers[j] != client->id[i])
				    				{
				    					flag = 1;
				    					break;
				    				}
				    			}
				    			V(semid);
				    			if(flag == 0)
				    			{
				    				flag1 = 1;
				    				n = write(newsocket_id,"Client not in the system.",255);
				    				break;
				    			}
				    			token = strtok(NULL, " ");
				    			j++;
				    		}

				    		if(flag1 == 1)
				    			continue;

				    		if(countMembers > 9)
				    		{
				    			n = write(newsocket_id,"Too many members. More than 10 not allowed",255);
				    			continue;
				    		}
				    		
				    		P(semid1);

				    		for(k = 0; k < 10; k++)
		    				{
		    					if(group->gid[k] == -1)
		    						break;
		    				}

		    				group->gid[k] = k+1;					//store the id
		    				group->admin[k] = client->id[i];		//admin info

		    				V(semid1);

		    				P(semid);

							client->NoOfGroups[i] += 1;
							for(m = 0; m < 10; m++)
					    	{
					    		if(client->group_id[i][m] == -1)
								{
					    			client->group_id[i][m] = group->gid[k];
									break;
								}
					    	}

					    	V(semid);
		    				bzero(buffer,255);

		    				for(j = 0; j < countMembers; j++)
		    				{
		    					cli_id = groupMembers[j];
		    					for(p = 0; p < 10; p++)
		    					{
		    						P(semid2);
		    						if(mesg->source[p] == -1)
		    						{
		    							mesg->source[p] = client->id[i];
		    							mesg->dest[p] = cli_id;
		    							sprintf(buffer,"Do you wish to join the group with id: %d\t and admin id: %d\n?. If yes then kindly execute the /joingroup command.",group->gid[k],group->admin[k]);
		    							strcpy(mesg->message[p],buffer);
		    							break;
		    						}
		    						V(semid2);
		    					}
		    				}
		    			}

		    			else
							n = write(newsocket_id,"Don't want any members?",255);
			    	}
			    	
			    	else if(strcmp(token,"/joingroup") == 0)
			    	{
			    		//new
			    		//send request to admin
			    		flag = 0;
			    		flag1 = 0;
			    		countMembers = 0;
			    		token = strtok(NULL, " ");
			    		if(token != NULL)
				    	{
				    		while(token != NULL)
				    		{
				    			countMembers++;
				    			flag = 0;
				    			flag1 = 0;
				    			groupJoin = atoi(token);
				    			P(semid1);
				    			for(k = 0; k < 10; k++)
				    			{
				    				if(groupJoin == group->gid[k])
				    				{
				    					flag = 1;
				    					break;
				    				}
				    			}
				    			V(semid1);
				    			if(flag == 0)
				    			{
				    				flag1 = 1;
				    				n = write(newsocket_id,"Group not in the system.",255);
				    				break;
				    			}
				    			token = strtok(NULL, " ");
				    		}

				    		if(flag1 == 1)
				    			continue;

				    		if(countMembers > 1)
				    		{
				    			n = write(newsocket_id,"One group at a time please.My processor is slow!",255);
				    			continue;
				    		}

				    		//check for duplicate group request

				    		P(semid);
				    		for(j = 0; j < 10; j++)
				    		{
				    			if(client->group_id[i][j] == groupJoin)
				    			{
				    				n = write(newsocket_id,"Hey you already are a member of this group!",255);
				    				break;
				    			}
				    		}
				    		V(semid);

				    		if(j != 10)
				    			continue;

		//During permission client-id corresponds to actual group index. Otherwise we wer storing gids in each client.
				    		if(client->permission[i][groupJoin-1] == 1)		//client is allowed to join the group
				    		{
				    			P(semid1);
				    			for(j = 0; j < 10; j++)
				    			{
				    				if(group->group_members[groupJoin-1][j] == -1)				//include that client in the group
				    				{
				    					group->group_members[groupJoin-1][j] = client->id[i];
				    					break;
				    				}
				    			}
				    			V(semid1);

				    			P(semid);

				    			client->NoOfGroups[i] += 1;

				    			for(j = 0; j < 10; j++)
				    			{
				    				if(client->group_id[i][j] == -1)
				    				{
				    					client->group_id[i][j] = groupJoin;				//include group Id in clients info
				    					break;
				    				}
				    			}
				    			V(semid);

				    		}

				    		else
				    		{
				    			sprintf(buffer,"Client %d wants to join group %d.Are you okay with it?",client->id[i],groupJoin);

					    		P(semid1);
					    		k = group->admin[groupJoin-1];
					    		V(semid1);

					    		P(semid2);

					    		for(j = 0; j < 10; j++)
								{
									if(mesg->source[j] == -1)
									{
										mesg->source[j] = client->id[i];
							    		mesg->dest[j] = k;
							    		strcpy(mesg->message[j],buffer);
							    		break;
									}
								}

					    		V(semid2);
					    	}
				    	}	

			    	}

			    	else if(strcmp(token,"/allow") == 0)
			    	{
			    		//allow <group-id> <client-id>
			    		//check if you are admin of that group, group-id is valid,client-id is valid. Then join. 
			    		flag = 0;
			    		token = strtok(NULL, " ");
			    		if(token != NULL)
			    		{
			    			gr_id = atoi(token);
			    			token = strtok(NULL, " ");
			    			cli_id = atoi(token);

			    			//if(token != NULL)				//more than two tokens
			    			//{
			    				//printf("%s",token);
			    				//n = write(newsocket_id,"Invalid command.",255);
			    				//continue;
			    			//}

			    			for(k = 0; k < 10; k++)			//check valid group-id
			    			{
			    				if(gr_id == group->gid[k])
			    				{
			    					flag = 1;
			    					break;
			    				}
			    			}

			    			if(flag == 0)
			    			{
			    				n = write(newsocket_id,"Group not in the system.",255);
			    				continue;
			    			}

			    			if(client->id[i] != group->admin[gr_id-1])	//check valid admin of the group
			    			{
			    				n = write(newsocket_id,"You are not authorized for this command.",255);
			    				continue;
			    			}
			    			flag = 0;
			    			for(k = 0; k < 10; k++)					//check valid client-id
			    			{
			    				if(cli_id == client->id[k])
			    				{
			    					flag = 1;
			    					break;
			    				}
			    			}

			    			if(flag == 0)
			    			{
			    				n = write(newsocket_id,"Client not in the system.",255);
			    				continue;
			    			}

			    			P(semid1);
				    		for(j = 0; j < 10; j++)
				    		{
				    			if(group->group_members[gr_id-1][j] == -1)				//include that client in the group
				    			{
				    				group->group_members[gr_id-1][j] = cli_id;
				    				break;
				    			}
				    		}
				    		V(semid1);

				    		P(semid);

				    		client->NoOfGroups[k] += 1;

				    		for(j = 0; j < 10; j++)
				    		{
				    			if(client->group_id[k][j] == -1)
				    			{
				    				client->group_id[k][j] = gr_id;				//include group Id in clients info
				    				break;
				    			}
				    		}
				    		V(semid);
			    		}
			    		else
							n = write(newsocket_id,"Allow what?",255);
			    	}

			    	else if(strcmp(token,"/activegroups") == 0)			 
			    	{
			    		j = 0;
			    		flag = 0;
			    		bzero(buffer,255);
			    		P(semid);
			    		while(client->group_id[i][j] != -1)
			    		{
			    			flag = 1;
			    			sprintf(buffer,"Active Groups you are part of is: %d",client->group_id[i][j]);
			    			n = write(client->sock[i],buffer,255);
			    			j++;
			    		}
			    		V(semid);

			    		if(flag == 0)
			    			n = write(newsocket_id,"Sorry you are not part of any group.",255);
			    	}
			    	
			    	else if(strcmp(token,"/activeallgroups") == 0)
			    	{
			    		bzero(buffer,255);
			    		P(semid1);
			    		sprintf(buffer,"Active Groups are: ",255);
			    		for(j = 0; j < 10; j++)
			    		{
				    		if(group->gid[j] != -1)
				    		{
				    			sprintf(buffer,"%d",group->gid[j]);
				    			n = write(client->sock[i],buffer,255);
				    		}
			    		}
			    	V(semid1);
			    	}
			    	
			    	else if(strcmp(token,"/sendfile") == 0)
			    	{
						//<client-id> <size>
						opts = fcntl(newsocket_id,F_GETFL);
						opts = opts & (~O_NONBLOCK);
						fcntl(newsocket_id, F_SETFL, opts);

						if (fcntl(newsocket_id,F_SETFL,opts) < 0) 
						{
       						perror("fcntl(F_SETFL)");
    					}

					
						flag = 0;
						flag1 = 0;
						FILE* fp;
						token = strtok(NULL, " ");
						tid = atoi(token);

						//check if valid client-id or group-id

						P(semid);

						for(j = 0; j < 10; j++)
						{
							if(client->id[j] == tid)
							{
								flag = 1;
								break;
							}
						}

						V(semid);

						P(semid1);

						if(flag == 0)
						{
							for(j = 0; j < 10; j++)
							{
								if(group->gid[j] == tid)
								{
									flag1 = 1;
									break;
								}
							}
						}

						V(semid1);

						if(flag ==0 && flag1 == 0)
						{
							sprintf(buffer,"Not a valid destination.");
							n = write(newsocket_id,buffer,255);
							continue;
						}

						token = strtok(NULL, " ");
						
						size = atoi(token);
						

						if(size<= 0)
						{
							sprintf(buffer,"Want me to send a empty file?");
							n = write(newsocket_id,buffer,255);
							continue;
						}

						//fp = fopen("tempo11.txt", "a+"); 
						fflush(stdin);
						//int mod=0;
						j = 0;
						while(j <= size)
						{
							n = read(newsocket_id,buffer,255);
							//printf("%d\n%s\n",n,buffer);
							//fwrite(buffer, n, 1, fp);
							//printf("%s",buffer);
        					//memset(buffer,0,255);
							j = j + n;
						}
						// fclose(fp);
						// char c = fgetc(fp); 
    		// 			while (c != EOF) 
    		// 			{ 
					 //        printf ("%c", c); 
					 //        c = fgetc(fp); 
					 //    } 

						status = fcntl(newsocket_id, F_SETFL, fcntl(newsocket_id, F_GETFL, 0) | O_NONBLOCK);

						// for(j = 0; j < 10; j++)
				  //   	{
				  //   		if(mesg->source[j] == -1)
				  //   		{
				  //   			mesg->source[j] = client->id[i];
				  //   			mesg->dest[j] = tid;
				  //   			strcpy(mesg->message[j],"receive");
				  //   			break;
				  //   		}
				  //   	}
						if(flag == 1)
						{
							P(semid2);
					    	for(j = 0; j < 10; j++)
					    	{
					    		if(mesg->source[j] == -1)
					    		{
					    			mesg->source[j] = client->id[i];
					    			mesg->dest[j] = tid;
					    			strcpy(mesg->message[j],buffer);
					    			break;
					    		}
					    	}
					    	V(semid2);
					    }

					    else if(flag1 == 1)
						{
							P(semid1);
							P(semid2);
									
							for(j = 0; j < 10; j++)
							{
								if(mesg->source[j] == -1)
								{
									mesg->source[j] = client->id[i];
							    	mesg->dest[j] = group->admin[tid-1];
							    	strcpy(mesg->message[j],buffer);
							    	break;
								}
							}
									
							for(k = 0; k < 10; k++)
							{
								if(group->group_members[tid-1][k] != -1)
								{
									for(j = 0; j < 10; j++)
									{
										if(mesg->source[j] == -1)
										{
											mesg->source[j] = client->id[i];
							    			mesg->dest[j] = group->group_members[tid-1][k];
							    			strcpy(mesg->message[j],buffer);
							    			break;
										}
									}

								}
							}

							V(semid1);
							V(semid2);
					    }
						
			    	}

			    	else
			    	{
			    		bzero(buffer,255);
			    		sprintf(buffer,"You entered a invalid command.Please enter the correct command.");
			    		n = write(newsocket_id,buffer,255);
			    	}

			    }

			    else
			    {
			    	//POLL
			    	bzero(buffer,255);
			    	for(j = 0; j < 10; j++)
			    	{
			    		if(mesg->dest[j] == client->id[i])
			    		{
			    			sprintf(buffer,"You received a message from %d:%s",mesg->source[j],mesg->message[j]);
			    			n = write(newsocket_id,buffer,255);
							mesg->source[j]=-1;
							mesg->dest[j]=-1;
							strcpy(mesg->message[j]," ");
			    		}
			    	}
			    }
		    }
		    
		    P(semid3);	    
	    	(*count)--;
    		V(semid3);
    		close(newsocket_id);
    		exit(0);

    	}

    }

    shmdt(client);
    shmdt(group);
    shmdt(count);
    shmdt(mesg);
    shmctl(id, IPC_RMID, 0);
    shmctl(id1, IPC_RMID, 0);
    shmctl(id2, IPC_RMID, 0);
    shmctl(id3, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    semctl(semid1, 0, IPC_RMID, 0);
    semctl(semid2, 0, IPC_RMID, 0);

	return 0;
}
