#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
	int fd;
	char cmd;
	char buff[32];
	do {
		cmd = getchar();
		getchar();
		switch (cmd) {
			case 'o':
				fd = open("./memdev", O_RDWR);
				break;
			case 'w':	
				printf("input the string pls:\n");
				memset(buff, 0, 32);
				fgets(buff, 32, stdin);
				write(fd, buff, strlen(buff) + 1);
				break;
			case 'r':
				memset(buff, 0, 32);
				read(fd, buff, sizeof(buff));
				buff[32] = '\0';
				printf("%s", buff);
				break;
			case 'c':
				close(fd);
				break;
			case 'i':
				ioctl(fd, 0);
				break;
			case 's':
				lseek(fd, 0, SEEK_CUR);
			default:
				;
		}
	}while(1);
	return 0;
}
