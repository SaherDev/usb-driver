
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#define MAX_PKT_SIZE 512
int main(int argc, char** argv)
{

	int fd;
	char buff[MAX_PKT_SIZE];
	char devicePath[100];
	
	
	//validate the input 
	if (argc != 3) {
		printf("SYNTAX ERROR: PROGRAM MINOR_NUMBER \"YOUR_MESSAGE\"\n");
		return -1;
	}

	
	strcpy(devicePath,"/dev/os2Ex_dev");
	strcat(devicePath, argv[1]);	
	fd=open(devicePath,O_RDWR);//open the file in /dev  
	
	if (fd == -1) {
		perror("error file open");
		return -1;
	}
				
	write(fd,argv[2],strlen(argv[2]));//write to the file
	read(fd, buff, sizeof(buff)); //read response from the file 
	printf("message from usb device: %s\n" , buff);//print the response
	close(fd); //close the file
	
	
}

