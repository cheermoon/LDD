#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	char op;
	int fd;
	int v;
	fd = open("/dev/led_drv", O_WRONLY);
	if (-1 == fd) {
		perror("/dev/led_drv");
		exit(EXIT_FAILURE);
	}
	do {
		printf("Input your operate pls[o/f]:");
		op = getchar();
		getchar();

		if ('o' == op || 'O' == op) {
			v = 1;
		} else {
			v = 0;
		}
		write(fd, &v, 4);
	} while('q' != op);

	close(fd);

	return 0;
}
