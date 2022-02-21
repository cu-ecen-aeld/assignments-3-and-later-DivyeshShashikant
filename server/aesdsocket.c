/*@author: Divyesh  Patel
 *@filename: aesdsocket.c
 *@brief: Assignment 5: Part 1: Native Socket Server
 *@references: https://beej.us/guide/bgnet/html/#bind
 *	       https://nullraum.net/how-to-create-a-daemon-in-c/	
*/

#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<syslog.h>
#include<arpa/inet.h>


#define PORT "9000"		//port	
#define BACKLOG 10		//pending connections
#define MAXSIZE 1024	


static void signal_handler(int signo);
int socketfd, newfd, fd; 			//listen on socketfd, new connection on newfd and fd (file descriptor) for opening new files)
const char *path = "/var/tmp/aesdsocketdata";

int main(int argc, char *argv[])
{
	int status,  sockstat, bindstat, listenstat, bytesread, ret;
	int yes = 1;
	
	pid_t pid;
	
	struct addrinfo hints;		     //instance to structure addrinfo
	struct addrinfo *servinfo;	     //points to addrinfo
	struct sockaddr_in store;	     //structure to be used wit IPv4 holds connection info
	socklen_t addr_size;		     //holds sockaddr_in size	
	
	memset(&hints, 0, sizeof(hints));    //make sure struct is empty
	hints.ai_family = PF_INET; 	     //use IPv4 i
	hints.ai_socktype = SOCK_STREAM;     //SOCK_STREAM or SOCK_DGRAM
	hints.ai_flags = AI_PASSIVE;  	     //fill up IP
	hints.ai_protocol = 0;		     //use protocol automatically

	char buf[MAXSIZE];		     //holds the packet received
	char send_buf[MAXSIZE];	     //holds the packet to be sent	
	char ipaddress[INET_ADDRSTRLEN];    //store the IP address of client
	
	memset(buf,0,MAXSIZE);		//clears buf
	memset(send_buf,0,MAXSIZE);	//clears send_buf
	
	//opens a connection to system logger and uses LOG_USER facility 
	openlog(NULL, 0, LOG_USER);
	
	//register signal_handler as our signal handler for SIGINT
	if(signal (SIGINT, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR,"Cannot handle SIGINT");
		closelog();
		exit(EXIT_FAILURE);
	}
	
	//register signal_handler as our signal handler for SIGTERM
	if(signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR,"Cannot handle SIGTERM");
		closelog();
		exit(EXIT_FAILURE);
	}
	

	//returns  one  or more addrinfo structures
	if((status = getaddrinfo(NULL, PORT, &hints, &servinfo))!=0)
	{
		fprintf(stderr, "getaddrinfo error: %s\n",gai_strerror(status));
		closelog();
		return -1;
	}

	//obtain socket file file descriptor
	socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(socketfd == -1)
	{
		perror("server: socket");
		closelog();
		return -1;
	}	
	
	//method to avoid the already in use message
	sockstat = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if(sockstat == -1)
	{
		perror("server: setsockopt");
		close(socketfd);
		closelog();
		return -1;
	}
	
	//assign an address to the socket
	bindstat = bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if(bindstat == -1)
	{
		perror("server: bind");
		close(socketfd);
		closelog();
		return -1;
	}
	
	//create daemon after successfull binding
	if(bindstat == 0)
	{
		
		if(argc ==2 && (!strcmp(argv[1],"-d")))
		{
			
			//create new process
			pid = fork();
			if(pid == -1)
			{
				syslog(LOG_ERR,"Fork Failed");
				closelog();
				exit(EXIT_FAILURE);
			}		
			else if(pid !=0)
			{	
				syslog(LOG_DEBUG,"Child pid is %d",pid);
				closelog();
				exit(EXIT_SUCCESS);
			}
			
			//create new session
			if(setsid() < 0)
			{
				exit(EXIT_FAILURE);
			}
			
			//catch, ignore and handle signals
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			
			
			//set new file permissions
			umask(0);
			
			//set the working directory to root directory
			if(chdir("/")== -1)
			{
				exit(EXIT_FAILURE);
			}
					
			//redirect fd's to 0,1,2 to /dev/null
			open("/dev/null", O_RDWR); //stdin
			dup(0);		   //stdout
			dup(0);		   //stderror
			
		}
	}

	freeaddrinfo(servinfo); //free memory when done with the data structure

	//listen for new connections on the socket
	listenstat= listen(socketfd, BACKLOG);
	if(listenstat == -1)
	{
		perror("server: listen");
		closelog();
		close(socketfd);
		return -1;
	}
	printf("*************Listening***************\n");	

	while(1)
	{
		addr_size = sizeof(store);

		newfd = accept(socketfd, (struct sockaddr *)&store, &addr_size);
		if(newfd == -1)
		{
			perror("server: accept");
			close(socketfd);
			closelog();
			return -1;
		}
		
		//get the IP address of the connected client
		if(store.sin_family == AF_INET)
		{
		inet_ntop(AF_INET, &store.sin_addr, ipaddress, INET_ADDRSTRLEN);
		printf("The IP address is %s\n",ipaddress);
		syslog(LOG_DEBUG,"Accepted connection from %s",ipaddress);
		}
		
		//open file to write the received packets in append mode
		fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0777);
		if(fd < 0)
		{
			printf("File not created\n");
			close(socketfd);
			close(newfd);
			closelog();
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("File created successfully in append mode\n");
		}	
		

	while(1)
	{
		//receive packets
		bytesread = recv(newfd, buf, sizeof(buf), 0);
		if(bytesread == -1)
		{
			perror("server: recv");
			close(socketfd);
			close(newfd);
			closelog();
			return -1;
		}
		
		//write packets in the file opened
		ret = write(fd, buf, bytesread);
		if(ret == -1)
		{
			perror("write failed");
			close(socketfd);
			close(newfd);
			close(fd);
			closelog();
			exit(EXIT_FAILURE);
		}
		
		if(buf[bytesread-1] == '\n')
			break;
			
	}
			
	close(fd);
	
	//open file for reading packets
	fd = open(path, O_RDONLY);
	
	//move file position to start
	lseek(fd, 0, SEEK_SET);
	while(1)
	{
		ret = read(fd, send_buf, MAXSIZE); 
		if(ret<0)
		{
			perror("read failed");
			close(socketfd);
			close(newfd);
			close(fd);
			closelog();
			exit(EXIT_FAILURE);
		}
		
		
		//send packets as they are read from the file
		int bytes_sent = send(newfd, send_buf, ret, 0);
		if(bytes_sent == -1)
		{
			perror("server: send");
			close(socketfd);
			close(newfd);
			close(fd);
			closelog();
			return -1;
		}
		
		if(!ret)
			break;
	}
	
	close(fd);
	syslog(LOG_DEBUG,"Closed Connection from %s",ipaddress);
	printf("file closed\n");
	closelog();
}

	return 0;
}	



//handler for SIGINT and SIGTERM
static void signal_handler(int signo)
{
	if((signo == SIGINT) || (signo == SIGTERM))
	{
		syslog(LOG_DEBUG,"Caught SIGNAL, exiting");
		
		
		if(shutdown(socketfd, SHUT_RDWR) == -1)
		{
			perror("Signal Handler : Shutdown");
			syslog(LOG_ERR,"Shutdown failure");
			closelog();
			exit(EXIT_FAILURE);
		}
		
		if(socketfd)
		{
			close(socketfd);
		}
		if(newfd)
		{
			close(newfd);
		}
		if(fd)
		{
			close(fd);
		}
		
		if(remove(path)==0)
		{
			syslog(LOG_DEBUG,"File removed successfully");
		}
		else
		{
			syslog(LOG_DEBUG,"File not deleted");
			closelog();
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}




