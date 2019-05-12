/* add '-a', '-l' option */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <grp.h>

/*
 * struct's pointer member var should not be assigned directly.
 * malloc first!
 */
struct file_info {
	char* modestr;
	int   link_num;
	char* user_id;
	char* group_id;
	off_t size;
	time_t mdf_time;
	char* name;
};

void 		do_ls(char *);
void 		print_files_name(char **,int);
void 		print_details(char**,int);
char* 		mode_tsf(mode_t mode);
char* 		userid_tsf(uid_t uid);
char* 		groupid_tsf(gid_t gid);
char*       time_tsf(time_t);
struct 		file_info* get_info(char **,int);
int 		compare_str(const void*, const void*);
static bool a_option = false;
static bool l_option = false;

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
					a_option = true;
					break;
				case 'l':
					l_option = true;
					break;
				default :
					fprintf(stderr, "unknown option\n");
					exit(1);
			}
		}
		if (optind == ac)
			do_ls(".");
		else 
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
			if (!a_option)
				if (dir->d_name[0] == '.')
					continue;
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

	if (l_option)
		return print_details(files_name, files_num);

	for (int i = 0; i < files_num; ++i) {
		printf("%s ", files_name[i]);
	}
	printf("\n");
}

void 
print_details(char **files_name, int files_num)
{
	int total_size = 0;
	struct file_info* files_info = get_info(files_name,files_num);
	for (int i = 0; i < files_num; ++i) {
		total_size += files_info[i].size;
	}
	printf("total: %d\n", total_size);
	for (int i = 0; i < files_num; ++i) {
		printf("%s ",files_info[i].modestr);
		printf("%d ",files_info[i].link_num);
		printf("%s ",files_info[i].user_id);
		printf("%s ",files_info[i].group_id);
		printf("%ld ",files_info[i].size);
		printf("%s ",time_tsf(files_info[i].mdf_time));
		printf("%s\n",files_info[i].name);
	}
}

struct file_info* 
get_info(char **files_name, int files_num)
{
	struct stat filest;
	struct file_info* files_info;
	files_info = malloc(sizeof(struct file_info)*files_num);
	for (int i = 0; i < files_num; ++i) {
		if (stat(files_name[i], &filest) == -1) {
			perror(files_name[i]);
			exit(1);
		}
		files_info[i].modestr = malloc(sizeof(char)*11);
		strcpy(files_info[i].modestr,mode_tsf(filest.st_mode));
		files_info[i].link_num = filest.st_nlink;		
		files_info[i].user_id = malloc(sizeof(char)*32); 
		strcpy(files_info[i].user_id, userid_tsf(filest.st_uid));
		files_info[i].group_id = malloc(sizeof(char)*32); 
		strcpy(files_info[i].group_id, groupid_tsf(filest.st_gid));
		files_info[i].size = filest.st_size;
		files_info[i].mdf_time = filest.st_mtime;
		files_info[i].name = malloc(sizeof(char)*256);
		strcpy(files_info[i].name, files_name[i]);
	}
	return files_info;
}
	
char* 
mode_tsf(mode_t mode)
{
	char* str = malloc(sizeof(char)*11);
	strcpy(str, "----------");
	if (S_ISDIR(mode)) str[0] = 'd';
	if (S_ISCHR(mode)) str[0] = 'c';
	if (S_ISBLK(mode)) str[0] = 'b';
	if (mode & S_IRUSR) str[1] = 'r';
	if (mode & S_IWUSR) str[2] = 'w';
	if (mode & S_IXUSR) str[3] = 'x';
	if (mode & S_IRGRP) str[4] = 'r';
	if (mode & S_IWGRP) str[5] = 'w';
	if (mode & S_IXGRP) str[6] = 'x';
	if (mode & S_IROTH) str[7] = 'r';
	if (mode & S_IWOTH) str[8] = 'w';
	if (mode & S_IXOTH) str[9] = 'x';
	return str;
}
char* 
userid_tsf(uid_t uid)
{
	return getpwuid(uid) -> pw_name;
}
char* 
groupid_tsf(gid_t gid)
{
	return getgrgid(gid) -> gr_name;
}
char*
time_tsf(time_t time)
{
	char *time_str = malloc(sizeof(char)*32);
	struct tm* gtime = gmtime(&time);
	strftime(time_str, 32, "%b %d %H:%M", gtime);
	return time_str;
}

int
compare_str(const void* s1, const void* s2)
{
	/* 
	 * s1,s2 both point to a pointer
	 * ues (char **) to transform
	 */
	return strcmp(*(char**)s1, *(char**)s2);
}
