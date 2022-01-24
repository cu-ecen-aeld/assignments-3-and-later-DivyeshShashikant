/*@author: Divyesh  Patel
 *@filename: writer.c
 *@brief: ASsignment 2: File Operations and Cross Compiler
*/

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
	//opens a connection to system logger and uses LOG_USER facility
	openlog(NULL,0,LOG_USER);

	//check is 3 arguments are provided or not
	if(argc!=3)
	{
		 syslog(LOG_ERR,"Invalid Number of arguments: %d",argc); 
		 syslog(LOG_ERR,"1.File Path"); 
		 syslog(LOG_ERR,"2.String to be entered in the file");	
 		 return 1;
	}

	//check if empty string is not passed as filename
	if(*argv[1]=='\0')
	{
		printf("Invalid File name \n");
		syslog(LOG_ERR,"Invalid File name");
		return 1;
	}
	
	int fd, wrbytes, closestat;

	//creates a new file or rewrites an existing one
	fd = creat (argv[1], 0777);
	
	//check for errors if file not created
	if(fd == -1)
	{
		syslog(LOG_ERR,"File cannot be created: %s",argv[1]);
		printf("File not created\n");	
		return 1;	
	}	
	else
	{
		syslog(LOG_DEBUG,"File created successfully: %s",argv[1]);
		printf("File created successfully\n");
		
		//write to file (fd)
		wrbytes = write(fd, argv[2], strlen(argv[2]));
		
		//check if write fail
		if(wrbytes == -1)
		{
			syslog(LOG_ERR,"Write not completed");
			return 1;
		}
		syslog(LOG_DEBUG,"Writing %s to %s",argv[2],argv[1]);
		printf("Writing to %s to %s\n",argv[2],argv[1]);
	
		//close fd
		closestat = close(fd);

		//check for errors if file failed to close
		if(closestat == -1)
		{
			syslog(LOG_ERR,"File not closed");
			return 1;
		}
		printf("close status %d\n",closestat);

	}
	
	//close log
	closelog();
	return 0;
}
