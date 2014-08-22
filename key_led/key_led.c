#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd_led, fd_key;
	char key_val;

	fd_led = open("/dev/m_led", O_WRONLY);
	if(-1 == fd_led) {
		perror("/dev/m_led");
		exit(EXIT_FAILURE);
	}

	fd_key = open("/dev/m_key", O_RDONLY);
	if(-1 == fd_key) {
		perror("/dev/m_key");
		close(fd_led);
		exit(EXIT_FAILURE);
	}

	do {
		read(fd_key, &key_val,1);
		write(fd_led, &key_val, 1);
	} while(0xf7 != key_val);
	
	close(fd_key);
	close(fd_led);
	
	return 0;
}

