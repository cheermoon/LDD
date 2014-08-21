#include <stdio.h>
#include <fcntl.h>
int main(int argc, char *argv[])
{
	char key_val = 0;
	int fd;

	fd = open("/dev/m_key", O_RDONLY);

	do {
		read(fd, &key_val, 1);
		printf("key value = 0x%x\n", key_val);
	} while(key_val);

	close(fd);
	return 0;
}
