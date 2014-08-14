#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	char op;
	int fd;
	int v;
	fd = open("/dev/led_drv", O_WRONLY);
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
