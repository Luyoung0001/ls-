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
#define PATH_MAX 1024
// 颜色宏
#define WHITE 0
#define BLUE 1
#define GREEN 2
#define RED 3
#define LBLUE 4
#define YELLOW 5
// 标记: -a、-l、-R、-t、-r、-i、-s 参数(向量分量)
#define a 0b1000000
#define l 0b0100000
#define R 0b0010000
#define t 0b0001000
#define r 0b0000100
#define I 0b0000010
#define s 0b0000001
// 函数声明
void tags_cal(int argc, char* argv[]);
void restored_ls(struct dirent* cur_item);
int get_color(struct stat buf);
void printf_name(char* name, int color);
void sort(char** filenames, int start, int end);
int partition(char** filenames, int start, int end);
void swap(char** s1, char** s2);
int compare(char* s1, char* s2);
void dostat(char*, char*);
void show_file_info(char*, char*, struct stat*);
void printf_name1(char* name, int color);
char* uid_to_name(uid_t);
char* gid_to_name(gid_t);
void mode_to_letters(int, char[]);
char* uid_to_name(uid_t);
// ********函数声明********
void do_i(char filename[]);
void do_s(char filename[]);
void do_name(char dirname[]);
void do_t(char dirname[]);
void sort_t(char* filenames[]);
void do_info(char filename[]);
// 设计可选参数向量
int Vec = 0;
// 存储dirname
char* dirname[2018];
// 存放文件夹参数的数量
int dirlen = 0;
// 文件名,初始化
char* filenames[2048];
// 某个目录中文件的个数
int file_cnt = 0;
int main(int argc, char* argv[]) {
    // 计算参数向量以及处理 dir参数(处理所有的输出)
    tags_cal(argc, argv);
    for (int i = 0; i < dirlen; i++) {
        do_name(dirname[i]);
    }
    return 0;
}
void do_t(char dirname[]) {
    // 获取各个文件时间戳
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort_t(filenames);
    }
}
void do_i(char filename[]) {
    struct stat info;
    if (stat(filename, &info) == -1)
        perror(filename);
    printf("%d  ", info.st_ino);
}
void do_s(char filename[]) {
    struct stat info;
    if (stat(filename, &info) == -1)
        perror(filename);
    long long size = info.st_size << 10;
    if (size <= 4)
        printf("4   ");
    else
        printf("%-4lld", size);
}
void do_name(char dirname[]) {
    int i = 0;
    int len = 0;
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL) {
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    } else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort(filenames, 0, file_cnt - 1);
        for (int j = 0; j < file_cnt; ++j) {
            struct stat info;
            if (stat(filenames[j], &info) == -1)
                perror(filenames[j]);
            int color = get_color(info);
            //*******打印名字*********
            // ls
            if ((Vec & a) == a) {
                printf_name(filenames[j], color);
            } else if ((Vec & l) == l)
                //*********************
                i++;
            if (i == 4) {
                printf("\n");
                i = 0;
            }
        }
    }
    printf("\n");
    closedir(dir_ptr);
    return;
}
void printf_name(char* name, int color) {
    if (color == GREEN) {
        printf("\033[1m\033[32m%-22s\033[0m", name);
    } else if (color == BLUE) {
        printf("\033[1m\033[34m%-22s\033[0m", name);
    } else if (color == WHITE) {
        printf("%-22s", name);
    } else if (color == LBLUE) {
        printf("\033[1m\033[36m%-22s\033[0m", name);
    } else if (color == YELLOW) {
        printf("\033[1m\033[33m%-22s\033[0m", name);
    }
}
int get_color(struct stat buf) {
    int color = 0;
    if (S_ISLNK(buf.st_mode)) {
        color = LBLUE;
    } else if (S_ISDIR(buf.st_mode)) {
        color = BLUE;
    } else if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode)) {
        color = YELLOW;
    } else if (buf.st_mode & S_IXUSR) {
        color = GREEN;
    }
    return color;
}
void sort(char** filenames, int start, int end) {
    if (start < end) {
        int position = partition(filenames, start, end);
        sort(filenames, start, position - 1);
        sort(filenames, position + 1, end);
    }
}
int partition(char** filenames, int start, int end) {
    if (!filenames)
        return -1;
    char* privot = filenames[start];
    while (start < end) {
        while (start < end && compare(privot, filenames[end]) < 0)
            --end;
        swap(&filenames[start], &filenames[end]);
        while (start < end && compare(privot, filenames[start]) >= 0)
            ++start;
        swap(&filenames[start], &filenames[end]);
    }
    return start;
}
void swap(char** s1, char** s2) {
    char* tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}
int compare(char* s1, char* s2) {
    if (*s1 == '.')
        s1++;
    if (*s2 == '.')
        s2++;
    while (*s1 && *s2 && *s1 == *s2) {
        ++s1;
        ++s2;
        if (*s1 == '.')
            s1++;
        if (*s2 == '.')
            s2++;
    }
    return *s1 - *s2;
}
void restored_ls(struct dirent* cur_item) {
    char* result = cur_item->d_name;
    filenames[file_cnt++] = result;
}
void do_info(char filename[]) {
    void mode_to_letters();
    char modestr[11];
    struct stat info;
    if (stat(filename, &info) == -1)
        perror(filename);
    mode_to_letters(info.st_mode, modestr);
    printf("%s ", modestr);
    printf("%4d ", (int)info.st_nlink);
    printf("%-8s ", uid_to_name(info.st_uid));
    printf("%-8s ", gid_to_name(info.st_gid));
    printf("%8ld ", (long)info.st_size);
    printf("%s ", ctime(&info.st_mtime));
    printf("\n");
}
void mode_to_letters(int mode, char str[]) {
    strcpy(str, "----------");
    if (S_ISDIR(mode))
        str[0] = 'd';
    if (S_ISCHR(mode))
        str[0] = 'c';
    if (S_ISBLK(mode))
        str[0] = 'b';
    if (mode & S_IRUSR)
        str[1] = 'r';
    if (mode & S_IWUSR)
        str[2] = 'w';
    if (mode & S_IXUSR)
        str[3] = 'x';

    if (mode & S_IRGRP)
        str[4] = 'r';
    if (mode & S_IWGRP)
        str[5] = 'w';
    if (mode & S_IXGRP)
        str[6] = 'x';

    if (mode & S_IROTH)
        str[7] = 'r';
    if (mode & S_IWOTH)
        str[8] = 'w';
    if (mode & S_IXOTH)
        str[9] = 'x';
}
char* gid_to_name(gid_t gid) {
    struct group *getgrgid(), *grp_ptr;
    static char numstr[10];
    if ((grp_ptr = getgrgid(gid)) == NULL) {
        sprintf(numstr, "%d", gid);
        return numstr;
    } else {
        return grp_ptr->gr_name;
    }
}
char* uid_to_name(gid_t uid) {
    struct passwd* getpwuid();
    struct passwd* pw_ptr;
    static char numstr[10];
    if ((pw_ptr = getpwuid(uid)) == NULL) {
        sprintf(numstr, "%d", uid);
        return numstr;
    } else {
        return pw_ptr->pw_name;
    }
}
void tags_cal(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        // 只接受以'-'开头的参数,其它参数要么错误,要么是文件夹名称或文件名
        if (argv[i][0] != '-') {
            char* tempdirname = (char*)malloc(sizeof(char) * 1024);
            strcpy(tempdirname, argv[i]);
            dirname[dirlen++] = tempdirname;
        } else {
            int len = strlen(argv[i]);
            for (int j = 1; j < len; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        Vec |= a;
                        break;
                    case 'l':
                        Vec |= l;
                        break;
                    case 'R':
                        Vec |= R;
                        break;
                    case 't':
                        Vec |= t;
                        break;
                    case 'r':
                        Vec |= r;
                        break;
                    case 'i':
                        Vec |= I;
                        break;
                    case 's':
                        Vec |= s;
                        break;
                    default:
                        break;
                }
            }
        }
        if (dirlen == 0) {
            dirlen = 1;
            dirname[0] = ".";
        }
    }
}
void sort_t(char* filenames[]) {
    // 创建 stat数组
    struct stat* stats = (struct stat*)malloc(sizeof(struct stat) * (file_cnt));
    // 提取 info
    for (int i = 0; i < file_cnt; i++) {
        if (filenames[i][0] == '.') {
            continue;
        }
        struct stat info;
        if (stat(filenames[i], &info) == -1) {
            perror(filenames[i]);
        }
        stats[i] = info;
    }
    printf("*************************\n");
    // 根据时间排序
    for (int i = 1; i < file_cnt - 1; i++) {
        for (int j = i + 1; j < file_cnt; j++) {
            if (stats[i].st_mtime> stats[j].st_mtime) {
                struct stat temp = stats[i];
                stats[i] = stats[j];
                stats[j] = temp;
            }
        }
    }
}
