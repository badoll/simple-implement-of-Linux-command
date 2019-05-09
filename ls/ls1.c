#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <string.h>


void do_ls(char *);
void print_files_name(char **,int);
int compare_str(const void*, const void*);

int
main(int ac, char *av[])
{
	if (ac == 1)
		do_ls(".");
	else {
		int ch;
		while ((ch = getopt(ac, av, "al")) != -1) {
			switch(ch)
			{
				case 'a':
					break;
				case 'l':
					break;
				default :
					fprintf(stderr, "unknown option\n");
			}
		}

		while(optind < ac) {
		/* 
		 * getopt() will set option on the left 
		 * and operand on the right in array arg[].
		 */
		 	printf("%s:\n", av[optind]);
			do_ls(av[optind++]);
		}
	}
	return 0;
}

void
do_ls(char *filename)
{
	DIR *dirptr;
	struct dirent *dir;
	char **files_name;
	files_name = (char **) malloc (sizeof(char *));
	int files_num = 0;
	if ((dirptr = opendir(filename)) != NULL) {
		while ((dir = readdir(dirptr)) != NULL) {
			files_name = (char **)realloc(files_name, sizeof(char*)*(files_num+1));
			files_name[files_num] = (char *) malloc (strlen(dir->d_name)+1);
			strcpy(files_name[files_num],dir->d_name);
			files_num++;
		}
		closedir(dirptr);
	} else
		fprintf(stderr, "open directory error\n");
	print_files_name(files_name, files_num);
	free(files_name);
}

void
print_files_name(char **files_name, int files_num)
{
	qsort(files_name, files_num, sizeof(char *), compare_str);
	for (int i = 0; i < files_num; ++i) 
		printf("%s ", files_name[i]);
	printf("\n");
}

int
compare_str(const void* s1, const void* s2)
{
	/* 
	 * because s1,s2 both point to a pointer
	 * ues (char **) to transform
	 */
	return strcmp(*(char**)s1, *(char**)s2);
}
