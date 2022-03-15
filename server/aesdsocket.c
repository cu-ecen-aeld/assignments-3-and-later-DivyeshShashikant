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
#include<sys/queue.h>
#include<pthread.h>
#include<stdbool.h>
#include<time.h>
#include<linux/fs.h>
#include <sys/times.h>


#define PORT "9000"     //port  
#define BACKLOG 10      //pending connections
#define MAXSIZE 1024    

#define USE_AESD_CHAR_DEVICE 1

#if(USE_AESD_CHAR_DEVICE ==1)
const char *path = "/dev/aesdchar";	//path to file aesdchar
#else
const char *path = "/var/tmp/aesdsocketdata";	//path to file aesdsocketdata
#endif

//structure that holds relevant data for the threads in linked list
typedef struct threads{
    pthread_t ids;          //store thread id of the accepted client
    int client_fd;          //store client fd of the accepted client
    bool complete;          //store the function completion status of the thread
    TAILQ_ENTRY(threads) nodes; //points to next node
}node_t;

// This typedef creates a head_t that makes it easy for us to pass pointers to
// head_t without the compiler complaining.
typedef TAILQ_HEAD(head_s, threads) head_t;
head_t head;


//Function prototypes
static void signal_handler(int signo);
void *thread_func(void* thread_param);
void *cleanup(void* clean_args);
void *timestamp_func(void *timestamp);
void _free_queue(head_t * head);
node_t* _fill_queue(head_t * head, const pthread_t threadid, const int threadclientid);



int count_nodes, count_nodes1;

int socketfd , newfd , fd ;   //listen on socketfd, new connection on newfd and fd (file descriptor) for opening new files)
//const char *path = "/var/tmp/aesdsocketdata";	//path to file aesdsocketdata

int status , sockstat , listenstat ,  thread_ret ,bindstat;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initialize mutex	
pthread_t timestampthread;	//holds threadid for the timestampthread
pthread_t threadc;		//holds threadid for the cleanup thread

int main(int argc, char *argv[])
{
    int yes = 1;
    
    //pid_t pid;
    
    struct addrinfo hints;           //instance to structure addrinfo
    struct addrinfo *servinfo = NULL;       //points to addrinfo
    struct sockaddr_in store;        //structure to be used wit IPv4 holds connection info
    socklen_t addr_size;             //holds sockaddr_in size   
    
    memset(&hints, 0, sizeof(hints));    //make sure struct is empty
    hints.ai_family = PF_INET;       //use IPv4 i
    hints.ai_socktype = SOCK_STREAM;     //SOCK_STREAM or SOCK_DGRAM
    hints.ai_flags = AI_PASSIVE;         //fill up IP
    hints.ai_protocol = 0;           //use protocol automatically

    char ipaddress[INET_ADDRSTRLEN];    //store the IP address of client
    
    memset(ipaddress, 0, sizeof ipaddress);
    
    //opens a connection to system logger and uses LOG_USER facility 
    openlog(NULL, LOG_CONS, LOG_USER);
    
    //register signal_handler as our signal handler for SIGINT
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        syslog(LOG_ERR,"Cannot handle SIGINT");
        closelog();
    }
    
    //register signal_handler as our signal handler for SIGTERM
    if(signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        syslog(LOG_ERR,"Cannot handle SIGTERM");
        closelog();
    }
    
  //register signal_handler as our signal handler for SIGTERM
  if((signal(SIGKILL, signal_handler) == SIG_ERR))
  {
      syslog(LOG_ERR,"Cannot handle SIGKILL");
      closelog();
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
        syslog(LOG_ERR,"server: socket");
        closelog();
        return -1;
    }   
    
    //method to avoid the already in use message
    sockstat = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(sockstat == -1)
    {
        perror("server: setsockopt");
        syslog(LOG_ERR,"server: setsockopt");
        close(socketfd);
        closelog();
        return -1;
    }
    
    //assign an address to the socket
    bindstat = bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(bindstat == -1)
    {
        perror("server: bind");
        syslog(LOG_ERR,"server: bind");
        close(socketfd);
        closelog();
        return -1;
    }
    
    remove(path);
    
    printf("Outside demon\n");
    //create daemon after successfull binding
    if(bindstat == 0)
    {
        
        if(argc ==2 && (!strcmp(argv[1],"-d")))
        {
            printf("inside demon\n");
            /*//create new process
            pid = fork();
            if(pid == -1)
            {
                syslog(LOG_ERR,"Fork Failed");
                closelog();
                //exit(//exit_FAILURE);
            }       
            else if(pid !=0)
            {   
                syslog(LOG_DEBUG,"Child pid is %d",pid);
                closelog();
                //exit(//exit_SUCCESS);
            }
            
            //create new session
            if(setsid() < 0)
            {
                //exit(//exit_FAILURE);
            }
            
            //catch, ignore and handle signals
            signal(SIGCHLD, SIG_IGN);
            signal(SIGHUP, SIG_IGN);
            
            
            //set new file permissions
            umask(0);
            
            //set the working directory to root directory
            if(chdir("/")== -1)
            {
                //exit(//exit_FAILURE);
            }
                    
            //redirect fd's to 0,1,2 to /dev/null
            open("/dev/null", O_RDWR); //stdin
            dup(0);        //stdout
            dup(0);        //stderror*/
            
            daemon(0,0);
            
        }
    }
    printf("left demon\n");
    
    freeaddrinfo(servinfo); //free memory when done with the data structure

    //listen for new connections on the socket
    listenstat= listen(socketfd, BACKLOG);
    if(listenstat == -1)
    {
        perror("server: listen");
        syslog(LOG_ERR,"server: listen");
        closelog();
        close(socketfd);
        return -1;
    }
    printf("*************Listening***************\n");  
    
    TAILQ_INIT(&head); //initialize the head
    
    
    int thread_clean = pthread_create(&threadc, NULL, cleanup, NULL);
    if(thread_clean!= 0)
    {
            perror("pthread_create cleanup");
            syslog(LOG_ERR,"pthread_create cleanup");
            return -1;
    }
    
#if(USE_AESD_CHAR_DEVICE !=1)

    int timestampret = pthread_create(&timestampthread, NULL, timestamp_func, NULL);
    if(timestampret != 0){
        perror("pthread_create timestamp");
        syslog(LOG_ERR, "Error: Timestamp thread creation failed: %s", strerror(errno));
        return -1;
    }

#endif    
    
    while(1)
    {
        addr_size = sizeof(store);
        newfd = accept(socketfd, (struct sockaddr *)&store, &addr_size);
        if(newfd == -1)
        {
            perror("server: accept");
            syslog(LOG_ERR,"server: accept");
            raise(SIGTERM);
            return -1;
        }
        
        //get the IP address of the connected client
        if(store.sin_family == AF_INET)
        {
            inet_ntop(AF_INET, &store.sin_addr, ipaddress, INET_ADDRSTRLEN);
            printf("The IP address is %s\n",ipaddress);
            syslog(LOG_DEBUG,"Accepted connection from %s",ipaddress);
        }
        
        node_t *current = NULL;
        current = _fill_queue(&head, 0, newfd);
        
        //create new thread for the accepted connection
        thread_ret = pthread_create(&(current->ids), NULL, thread_func, (void *)current);
        if(thread_ret)
        {
                perror("pthread_create");
                exit(EXIT_FAILURE);
        }
        
        printf("Newfd from main is %d\n",newfd);
        printf("threadID from main is %ld\n",current->ids);
        
    }     
    closelog();
    close(socketfd); 
    close(newfd);
    return 0;
}   

void *thread_func(void* thread_param)
{
    struct threads *thread_func_args = thread_param;
    
    int bytesread, ret;        
            
    char buf[MAXSIZE];           //holds the packet received
    char send_buf[MAXSIZE];      //holds the packet to be sent
    
    memset(buf,0,MAXSIZE);      //clears buf
    memset(send_buf,0,MAXSIZE); //clears send_buf
    
    
    pthread_mutex_lock(&mutex);
    //open file to write the received packets in append mode
    fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0777);
    if(fd < 0)
    {
        printf("File not created\n");
        closelog();
    }
    else
    {
        printf("File created successfully in append mode\n");
    }   
    
    
    while(1)
    {
        //receive packets
        bytesread = recv(thread_func_args->client_fd, buf, sizeof(buf), 0);
        if(bytesread == -1)
        {
            pthread_mutex_unlock(&mutex);
            perror("server: recv");
            syslog(LOG_ERR,"server: recv");
            pthread_exit(NULL);
        }
        
        printf("received buffer\n");        
        //write packets in the file opened
        ret = write(fd, buf, bytesread);
        if(ret == -1)
        {
            pthread_mutex_unlock(&mutex);
            perror("write failed");
            syslog(LOG_ERR,"write failed");
        }
        
        if(buf[bytesread-1] == '\n')
            break;
            
    }
    
    //move file position to start
    lseek(fd, 0, SEEK_SET);
    while(1)
    {
        ret = read(fd, send_buf, MAXSIZE); 
        if(ret<0)
        {
            perror("read failed");
            syslog(LOG_ERR,"read failed");
            pthread_exit(NULL);
        }
        
        //send packets as they are read from the file
        int bytes_sent = send(thread_func_args->client_fd, send_buf, ret, 0);
        if(bytes_sent == -1)
        {
            perror("server: send");
            syslog(LOG_ERR,"server: send");
            pthread_exit(NULL);
        }
        
        
        if(!ret)
        {
            break;
        }
    }
    printf("send buffer\n");
    thread_func_args->complete = 1;
    close(fd);
    shutdown(thread_func_args->client_fd, SHUT_RDWR);
    pthread_mutex_unlock(&mutex);
    printf("file closed\n");
    return NULL;
}


void *cleanup(void* clean_args)
{
    printf("entered cleanup function\n");

    while(1)
    {
        node_t *tmp = NULL;
        TAILQ_FOREACH(tmp, &head, nodes)
        {
            if(tmp->complete == 1)
            {
                syslog(LOG_DEBUG,"Cleaning thread %ld with complete value %d", tmp->ids, tmp->complete);
                
                if((pthread_join(tmp->ids, NULL))!=0)
                {
                    syslog(LOG_ERR,"pthread_join failed from cleanup");
                    printf("pthread_join error\n");
                }
                TAILQ_REMOVE(&head, tmp, nodes);
                free(tmp);
                tmp = NULL;
                count_nodes1++;
                printf("nodes deletes from threads %d\n",count_nodes);
                break;    
            }
        }
        usleep(10);
    }       
    return NULL;        
}

#if(USE_AESD_CHAR_DEVICE !=1)
void *timestamp_func(void *timestamp)
{
    while(1)
    {
       char outstr[200] ={0};
           time_t t;
           struct tm *tmp;
           
           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
           }

           if (strftime(outstr, sizeof(outstr),"timestamp: %a %d %b %Y %T", tmp) == 0) 
           {
               perror("strftime error");
               syslog(LOG_ERR,"strftime error");
               //exit(//exit_FAILURE);
           }
           
           strcat(outstr,"\n");

           printf("Result string is %s\n", outstr);
           
       
        pthread_mutex_lock(&mutex);
        //open file to write the received packets in append mode
        fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0777);
        if(fd < 0)
        {
            printf("File not created for timestamp\n");
        }
        else
        {
            printf("File created successfully in append mode for timestamp\n");
        }   

        //write packets in the file opened
        int stampret = write(fd, outstr, strlen(outstr));
        if(stampret == -1)
        {
            perror("write failed");
            syslog(LOG_ERR,"write failed");
        }
        close(fd);
        pthread_mutex_unlock(&mutex);   

        sleep(10);
    }

}

#endif

    
//handler for SIGINT and SIGTERM
static void signal_handler(int signo)
{
        syslog(LOG_DEBUG,"Caught SIGNAL, //exiting");
        
        printf("Caught SIGNAL, //exiting\n");
        
        node_t *handle = NULL;
        TAILQ_FOREACH(handle, &head, nodes)
        {
            if((shutdown(handle->client_fd, SHUT_RDWR)) == -1)
            {
                syslog(LOG_ERR,"Shutdown failure from threads");
            }
            
            if((pthread_kill(handle->ids, SIGKILL))!=0)
            {
                syslog(LOG_ERR,"Unable to kill thread");
            }
            
        }
        
        _free_queue(&head);
        printf("Hi\n");
        pthread_mutex_destroy(&mutex);
        
        syslog(LOG_DEBUG,"Mutex destroyed");
        if(shutdown(socketfd, SHUT_RDWR) == -1)
        {
            perror("Signal Handler : Shutdown");
            syslog(LOG_ERR,"Shutdown failure");
            closelog();
        }
        if((pthread_kill(threadc,SIGKILL)!=0))
        {
            syslog(LOG_ERR,"threadc FAILURE");
        }
        if((pthread_kill(timestampthread,SIGKILL))!=0)
        {
            syslog(LOG_ERR,"timestampthread FAILURE");
        }
        
        if(socketfd)
        {
            if(close(socketfd))
            {
                syslog(LOG_ERR,"socketfd FAILURE");
            }
            
        }
         if(newfd)
         {
		  if(close(newfd))
		  {
		      syslog(LOG_ERR,"newfd FAILURE");
		  }
         }
        if(fd)
        {
            if(close(fd))
            {
                syslog(LOG_ERR,"fd FAILURE");
            }
        }
        
        unlink(path);
        closelog();
}

// manages the linked list of given threads
node_t* _fill_queue(head_t * head, const pthread_t threadid, const int threadclientid)
{
    struct threads *e = malloc(sizeof(struct threads));
    if(e == NULL)
    {
        printf("Malloc Unsuccessfull\n");
        exit(EXIT_FAILURE);
    }
    e->ids = threadid;
    e->client_fd = threadclientid;
    e->complete = 0;
    TAILQ_INSERT_TAIL(head,e,nodes);
    e = NULL;

    count_nodes++;
    printf("node created for threads are %d\n",count_nodes);
    return TAILQ_LAST(head, head_s);
}

// Removes all of the elements from the queue before free()ing them
void _free_queue(head_t * head)
{
    struct threads *e = NULL;
    while(!TAILQ_EMPTY(head))
    {
        e = TAILQ_FIRST(head);
        TAILQ_REMOVE(head,e,nodes);
        free(e);
        e = NULL;
    }
    
}
