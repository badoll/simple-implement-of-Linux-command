#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define BUFSIZE 4096

char buf[BUFSIZE];
off_t locate(int, int);
void print_file(int, off_t);

int 
main(int ac, char *av[])
{
	int fd, n_line = 10;
	if (ac < 2) {
		fprintf(stderr, "usage: tail [option] source");
		exit(1);
	}
	/*
	char argument[10];
	while (--ac) {
		if (strcmp(*++av, "-n") == 0) {
			strcpy(argument, "-n");
			n_line = atoi(*++av);
			ac--;
		} else {	
			if ((fd = open(*av, O_RDONLY)) == -1) {
				fprintf(stderr, "Open file error");
				exit(1);
			}
		}
	}
	*/
	int ch;
	while ((ch = getopt(ac, av, "n:")) != -1)
		switch(ch)
		{
			case 'n':
				n_line = atoi(optarg);
				break;
			default:
				fprintf(stderr, "error option\n");
				exit(1);
		}
	printf("line:%d\n",n_line);
	if ((fd = open(av[optind], O_RDONLY)) == -1) {
		fprintf(stderr, "open error\n");
		exit(1);
	}
	
	print_file(fd, locate(fd, n_line));

	return 0;
}

/*
 * locate the offset to the end of the first position of the last n lines
 */
off_t
locate(int fd, int n_line)
{
	int n_count = 0; //count '\n'
	int len;
	off_t offs_cur = 0;
	off_t offs_pre = 0;
	off_t offs_to_end = 0;
	memset(buf, 0, BUFSIZE);
	struct stat st;
	fstat(fd, &st);
	printf("file_size:%ld\n",st.st_size);
	if (st.st_size > BUFSIZE) {
	/* source file's size is bigger than buffer size*/
		while ((offs_cur = lseek(fd, offs_cur - BUFSIZE, SEEK_END)) != -1) {
			if ((len = read(fd, buf, BUFSIZE)) > 0) {
				for (int i = len - 1; i >= 0; --i)
					if (buf[i] == '\n')
						if (++n_count == n_line + 1) {
							offs_to_end += len - (i + 1);
							printf("offs_to_end:%ld\n",offs_to_end);
							break;
						}
			} else 
				break;
			if (n_count == n_line + 1) break;
			offs_to_end += len;
			offs_pre = offs_cur;
			memset(buf,0,BUFSIZE);
		}
		printf("offs_cur:%ld\n",offs_cur);
	} else {
		offs_cur = -1;
		offs_pre = st.st_size;
		printf("file_size<BUFSIZE\n");
	}

	
	if (offs_cur == -1) {
	/* size of the remaining file that haven't read is smaller than buffer size*/
		lseek(fd, 0, SEEK_SET);
		if ((len = read(fd, buf, (size_t)offs_pre)) > 0) {
			printf("len:%d\n",len);
			for (int i = len - 1; i >= 0; --i) 
				if (buf[i] == '\n')
					if (++n_count == n_line + 1) {
						offs_to_end += len - (i + 1);
						printf("add offset:%ld\n",offs_to_end);
						break;
					}
		}
	}

	printf("offs_to_end:%ld\n",offs_to_end);
	return offs_to_end;
}

void
print_file(int fd, off_t offset)
{	
	/* display data from the offs_to_end to the end. */
	int len;
	memset(buf,0,BUFSIZE);
	if (lseek(fd, -offset, SEEK_END) != -1)
		while ((len = read(fd, buf, BUFSIZE)) > 0) {
			printf("%s",buf);
			printf("len=%d\n",len);
			if (len < BUFSIZE)
				break;
		}
}
