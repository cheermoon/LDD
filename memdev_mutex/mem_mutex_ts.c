/*  
 memts.c : ldd

	Author : moon.cheng.2014@gmail.com  
	Date   : 2014-08-05
	Version: 1.0
					
	This program is a testing program for charactor drivers.You can use
	it to open/close/read/write/llseek/ioctl a charactor devices. The 
	origin purpose for this program is to test the functions in memdev.c
				
							
	If you find some bugs or you have some advices. Send me a email pls!
							
	This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
									
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

void m_read(int fd)
{
	int r_number;
	char buff[33];

	memset(buff, 0, 32);

	printf("input the number of bytes you want to read pls[1-32]:");
	scanf("%d", &r_number);
	getchar();
	
	if (r_number <= 0 || r_number > 32) {
		r_number = 4;
	}
	r_number = read(fd, buff, r_number);

	buff[r_number+1] = '\0';
	printf("%s\n", buff);
}

void m_write(int fd)
{
	char buff[32];
	printf("input the string you want to write pls:");
	memset(buff, 0, 32);
	fgets(buff, 32, stdin);
	write(fd, buff, strlen(buff)-1);
}

void m_seek(int fd)
{
	char seek_base;
	int offset;
	printf("syntax:_N N is the offset number\n");
	printf("       sN---seek from the begin\n");
	printf("       cN---seek from the current\n");
	printf("       eN---seek from the end\n");
	printf("input where to seek pls:");

	seek_base = getchar();
	scanf("%d", &offset);
	getchar();

	switch (seek_base){
		case 's':
			lseek(fd, offset, SEEK_SET);
			break;
		case 'c':
			lseek(fd, offset, SEEK_CUR);
			break;
		case 'e':
			lseek(fd, offset, SEEK_END);
			break;
		default:
			printf("Check you input!\n");
	}
}

int main(void)
{
	int fd = -1;
	char cmd;
	do {
		printf("[o-open|w-write|r-read|c-clear|s-seek|q-quit]:");
		cmd = getchar();
		getchar();
		switch (cmd) {
			case 'o':
				if (-1 == fd) 
					fd = open("./memdev_mutex", O_RDWR);
				break;
			case 'w':
				if (-1 == fd) {
					printf("you should run command o first!\n");
					break;
				}
				m_write(fd);	
				break;
			case 'r': 
				if (-1 == fd) {
					printf("you should run command o first!\n");
					break;
				}
				m_read(fd);
				break;
			case 'q':
				if (-1 != fd)
					close(fd);
				break;
			case 'c':
				if (-1 == fd) {
					printf("you should run command o first!\n");
					break;
				}
				ioctl(fd, 0);
				break;
			case 's':
				if (-1 == fd) {
					printf("you should run command o first!\n");
					break;
				}
				m_seek(fd);
				break;
			default:
				;
		}
	}while('q' != cmd);
	return 0;
}
