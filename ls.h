#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define PATH_MAX 200
// 参数
void do_ls(char[]);      // -al
void do_ls1(char[]);     // -l
void do_ls2(char[]);     // -a
void do_ls3(char[]);     // ls
void do_ls4(char[]);     // ls
void do_ls5(char[]);     // ls -i
void do_ls6(char[]);     // ls -ial
void ls_R(char path[]);  // ls -R

void mode_to_letters(int, char[]);
void dostat(char*, char*);
void show_file_info(char*, char*, struct stat*);
char* uid_to_name(uid_t);
char* gid_to_name(gid_t);
void match(int argc, char* argv[]);
void restored_ls(struct dirent*);
void error_handle(const char*);
// 字典序
void swap(char** s1, char** s2);
int compare(char* s1, char* s2);
int partition(char** filenames, int start, int end);
void sort(char** filenames, int start, int end);
// 文件名字颜色
int get_color(struct stat buf);
void printf_name(char* name, int color);
void printf_name1(char* name, int color);
// 颜色参数
#define WHITE 0
#define BLUE 1
#define GREEN 2
#define RED 3
#define LBLUE 4
#define YELLOW 5
int has_a = 0;
int has_l = 0;
int has_al = 0;
int has_i = 0;
int has_ial = 0;
int has_il = 0;
int has_ai = 0;
int has_R = 0;
int has_aR = 0;
int has_Rl = 0;
int has_iR = 0;
int has_aRl = 0;
int has_r = 0;
int has_ar = 0;
int has_s = 0;
// 存放数组名的数组
char* filenames[4096];
int file_cnt = 0;  // 目录中文件个数