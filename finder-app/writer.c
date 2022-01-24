#include<stdio.h>
#include<syslog.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>


int main(int argc, char* argv[])
{
	openlog(NULL,0,LOG_USER);

	if(argc!=3)
	{
		 syslog(LOG_ERR,"Invalid Number of arguments: %d",argc); 
		 syslog(LOG_ERR,"1.File Path"); 
		 syslog(LOG_ERR,"2.String to be entered in the file");	
 		 return 1;
	}

	if(*argv[1]=='\0')
	{
		printf("Invalid File name \n");
		syslog(LOG_ERR,"Invalid File name");
		return 1;
	}
	
	int fd, wrbytes, closestat;
	fd = creat (argv[1], 0777);
	if(fd == -1)
	{
		syslog(LOG_ERR,"File cannot be created: %s",argv[1]);
		printf("file not created\n");	
		return 1;	
	}	
	else
	{
		syslog(LOG_DEBUG,"File created successfully: %s",argv[1]);
		printf("File created successgfully\n");
		wrbytes = write(fd, argv[2], strlen(argv[2]));
		if(wrbytes == -1)
		{
			syslog(LOG_ERR,"Write not completed");
			return 1;
		}
		syslog(LOG_DEBUG,"Writing %s to %s",argv[2],argv[1]);
		printf("Writing to %s to %s\n",argv[2],argv[1]);
		printf("number of byters written %d\n",wrbytes);
		closestat = close(fd);
		if(closestat == -1)
		{
			syslog(LOG_ERR,"File not closed");
			return 1;
		}
		printf("%d close status\n",closestat);

	}
	

	closelog();
	return 0;
}
