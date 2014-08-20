#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>

int main(int argc, char *argv[])
{
	int fd;
	char c;
	fd_set rd_fds, wr_fds;

	if (2 != argc) {
		printf("Syntax: cmd filename\n");
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (-1 == fd) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	printf("Press [ENTER] poll\nPress [q] to quit:");
	getchar();
	do {
		FD_ZERO(&rd_fds);
		FD_ZERO(&wr_fds);
		FD_SET(fd, &rd_fds);
		FD_SET(fd, &wr_fds);

		select(fd+1, &rd_fds, &wr_fds, NULL, NULL);

		if (FD_ISSET(fd, &rd_fds))
			printf("You can READ!!");
		if (FD_ISSET(fd, &wr_fds))
			printf("You can WRITE!!");

		c = getchar();
	} while('q' != c);

	close(fd);

	return 0;
}
