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
/*需要实现以下命令:
 *ls
 *ls -a
 *ls -l
 *ls -i
 *ls -t
 *ls -*
 ********
 *ls .
 *ls /
 *ls *
 */
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

// do_ls*
void do_ls(char filename[]);
void do_ls_a(char filename[]);
void do_ls_r(char filename[]);
void do_ls_l(char filename[]);
void do_ls_al(char filename[]);
void do_ls_i(char filename[]);
void do_ls_li(char filename[]);

// 标记: -a、-l、-R、-t、-r、-i、-s 参数(向量分量)
int tag_a = 0b1000000;
int tag_l = 0b0100000;
int tag_R = 0b0010000;
int tag_t = 0b0001000;
int tag_r = 0b0000100;
int tag_i = 0b0000010;
int tag_s = 0b0000001;
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
    if (argc == 1) {
        do_ls(".");
    } else {
        // 开始应对其它情况
        /*  ls -a -l
            ls -a /home
            ls -alR /
            ls -t
            ls -ai -t /home
            ls -i /home /etc*/
        // 可能的组合有以下几种情况,且暂时只实现这几种组合
        /*ls -a
        ls -l
        ls -al

        ls -i
        ls -ai
        ls -ail
        ls -il

        ls -r
        ls -ar
        ls -s
        另外可以选择加上t, R
        */
        // a 或者 ar被选中,s可选
        if (Vec == 0b1000000 || Vec == 0b1000100 || Vec == 0b1000001 ||
            Vec == 0b1000101) {
            for (int i = 0; i < dirlen; i++) {
                do_ls_a(dirname[i]);
            }
        } else if (Vec == 0b0000100 || Vec == 0b0000101) {
            // ls -r,s可选
            for (int i = 0; i < dirlen; i++) {
                do_ls_r(dirname[i]);
            }
            // ls -l
        } else if (Vec == 0b0100000) {
            for (int i = 0; i < dirlen; i++) {
                do_ls_l(dirname[i]);
            }
        } else if (Vec == 0b1100000 || Vec == 0b1100010) {
            // ls -al, i可选
            for (int i = 0; i < dirlen; i++) {
                do_ls_al(dirname[i]);
            }
        } else if (Vec == 0b0000010 || Vec == 0b0000011 || Vec == 0b1000010 ||
                   Vec == 0b1000011) {
            // ls -i,s,a可选
            for (int i = 0; i < dirlen; i++) {
                do_ls_i(dirname[i]);
            }
        } else if (Vec == 0b0100010) {
            // ls -il
            for (int i = 0; i < dirlen; i++) {
                do_ls_li(dirname[i]);
            }
        } /*else if (has_at == 1) {
            // ls -aR
            ls_t(name);
            else if (has_aR == 1) {
                // ls -aR
                ls_R(name);
            }
            else if (has_R == 1) {
                // ls -R
                ls_R(name);
            }
            else if (has_aRl == 1 || has_Rl == 1) {
                // ls -aRl, ls -Rl
                ls_R(name);
            }
            else if (has_s == 1) {
                // ls -s
                do_ls2(name);
            }
            else {
            }
        }*/
        return 0;
    }
}
//
void tags_cal(int argc, char* argv[]) {
    if (argc == 2) {
        dirname[0] = ".";
        dirlen = 1;
    }
    for (int i = 1; i < argc; i++) {
        // 只接受以'-'开头的参数,其它参数要么错误,要么是文件夹名称
        if (argv[i][0] != '-') {
            char* tempdirname = (char*)malloc(sizeof(char) * 1024);
            strcpy(tempdirname, argv[i]);
            dirname[dirlen++] = tempdirname;
        } else {
            int len = strlen(argv[i]);
            for (int j = 1; j < len; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        Vec |= tag_a;
                        break;
                    case 'l':
                        Vec |= tag_l;
                        break;
                    case 'R':
                        Vec |= tag_R;
                        break;
                    case 't':
                        Vec |= tag_t;
                        break;
                    case 'r':
                        Vec |= tag_r;
                        break;
                    case 'i':
                        Vec |= tag_i;
                        break;
                    case 's':
                        Vec |= tag_s;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void do_ls(char dirname[]) {
    int i = 0;
    struct dirent* direntp;
    DIR* dir_ptr = opendir(dirname);
    if (dir_ptr == NULL) {
        fprintf(stderr, "ls:cannot open %s\n", dirname);
    } else {
        // 存储文件名
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        int j = 0;
        for (j = 0; j < file_cnt; ++j) {
            // 如果文件名包括'.',就忽略,因为'.'不参与命令"ls"
            if (filenames[j][0] == '.')
                continue;
            struct stat info;
            // 如果文件名错误
            if (stat(filenames[j], &info) == -1)
                perror(filenames[j]);
            // 处理颜色
            int color = get_color(info);
            printf_name(filenames[j], color);
            // 换行
            i++;
            if (i == 4) {
                printf("\n");
                i = 0;
            }
        }
    }
    printf("\n");
    closedir(dir_ptr);
}
void restored_ls(struct dirent* cur_item) {
    char* result = cur_item->d_name;
    filenames[file_cnt++] = result;
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
void do_ls_a(char dirname[]) {
    // printf("%s\n",dirname);
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
        int j = 0;
        // a r
        if ((Vec & 0b1000100) == 0b1000100) {
            for (j = file_cnt - 1; j >= 0; --j) {
                struct stat info;
                if (stat(filenames[j], &info) == -1)
                    perror(filenames[j]);
                int color = get_color(info);
                // s
                if ((Vec & 0b0000001) == 1) {
                    long long size = info.st_size / 1024;
                    if (size <= 4)
                        printf("4   ");
                    else
                        printf("%-4lld", size);
                }
                printf_name(filenames[j], color);
                i++;
                if (i == 4) {
                    printf("\n");
                    i = 0;
                }
            }
            printf("\n");
            closedir(dir_ptr);
            // a
        }
        for (j = 0; j < file_cnt; ++j) {
            struct stat info;
            if (stat(filenames[j], &info) == -1)
                perror(filenames[j]);
            int color = get_color(info);
            // s
            if ((Vec & 0b0000001) == 1) {
                long long size = info.st_size / 1024;
                if (size <= 4)
                    printf("4   ");
                else
                    printf("%-4lld", size);
            }
            printf_name(filenames[j], color);
            i++;
            if (i == 4) {
                printf("\n");
                i = 0;
            }
        }
    }
    printf("\n");
    closedir(dir_ptr);
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

void do_ls_r(char dirname[]) {
    int i = 0;
    struct dirent* direntp;
    DIR* dir_ptr = opendir(dirname);
    if (dir_ptr == NULL) {
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    } else {
        // 存储文件名
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        // 对文件名进行排序
        sort(filenames, 0, file_cnt - 1);
        // 倒序排
        for (int j = file_cnt - 1; j >= 0; j--) {
            // 如果文件名包括'.',就忽略,因为'.'不参与ls -l
            if (filenames[j][0] == '.') {
                continue;
            }
            // 获取某个文件的 stat 信息
            struct stat info;
            if (stat(filenames[j], &info) == -1) {
                perror(filenames[j]);
            }
            // 处理颜色
            int color = get_color(info);
            // 打印块大小
            // s可选
            if ((Vec & 0b0000001) == 1) {
                long long size = info.st_size > 10;
                if (size <= 4) {
                    printf("4   ");
                } else {
                    printf("%-4lld", size);
                }
            }
            // 打印文件名
            printf_name(filenames[j], color);
            // 换行
            i++;
            if (i == 4) {
                printf("\n");
                i = 0;
            }
        }
    }
    printf("\n");
    closedir(dir_ptr);
}
void do_ls_l(char dirname[]) {
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort(filenames, 0, file_cnt - 1);
        int j = 0;
        for (j = 0; j < file_cnt; ++j) {
            if (filenames[j][0] == '.')
                continue;
            char temp1[PATH_MAX];
            sprintf(temp1, "%s/%s", dirname, filenames[j]);
            dostat(temp1, filenames[j]);
        }

        closedir(dir_ptr);
    }
}
void do_ls_al(char dirname[]) {
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort(filenames, 0, file_cnt - 1);
        int j = 0;
        for (j = 0; j < file_cnt; ++j) {
            // a可选
            if ((Vec & 0b1000000) == 0) {
                if (filenames[j][0] == '.')
                    continue;
            }
            char temp1[PATH_MAX];
            sprintf(temp1, "%s/%s", dirname, filenames[j]);
            dostat(temp1, filenames[j]);
        }

        closedir(dir_ptr);
    }
}
void do_ls_i(char dirname[]) {
    int i = 0;
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "ls1:cannot open %s\n", dirname);
    else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort(filenames, 0, file_cnt - 1);
        int j = 0;
        for (j = 0; j < file_cnt; ++j) {
            // a 可选
            if ((Vec & 0b1000000) != 0b1000000) {
                if (filenames[j][0] == '.')
                    continue;
            }
            struct stat info;
            if (stat(filenames[j], &info) == -1)
                perror(filenames[j]);
            printf("%d  ", info.st_ino);
            int color = get_color(info);
            // s可选
            if ((Vec & 0b0000001) == 1) {
                long long size = info.st_size / 1024;
                if (size <= 4)
                    printf("4   ");
                else
                    printf("%-4lld", size);
            }
            printf_name(filenames[j], color);
            i++;
            if (i == 4) {
                printf("\n");
                i = 0;
            }
        }

        closedir(dir_ptr);
        printf("\n");
    }
}
void dostat(char* path, char* filename) {
    struct stat info;
    if (stat(path, &info) == -1)
        perror(path);
    else
        show_file_info(path, filename, &info);
}
void show_file_info(char* path, char* filename, struct stat* info_p) {
    char *uid_to_name(), *ctime(), *git_to_name(), *filemode();
    void mode_to_letters();
    char modestr[11];
    struct stat info;
    if (stat(path, &info) == -1)
        perror(path);
    int color = get_color(info);
    if ((Vec & 0b0000001) == 1) {
        long long size = info_p->st_size / 1024;
        if (size <= 4)
            printf("4   ");
        else
            printf("%-4lld", size);
    }
    mode_to_letters(info_p->st_mode, modestr);
    if ((Vec & 0b1100010) == 0b1100010 || (Vec & 0b0100010) == 0b0100010)
        printf("%ul ", info_p->st_ino);
    printf("%s ", modestr);
    printf("%4d ", (int)info_p->st_nlink);
    printf("%-8s ", uid_to_name(info_p->st_uid));
    printf("%-8s ", gid_to_name(info_p->st_gid));
    printf("%8ld ", (long)info_p->st_size);
    printf("%.12s ", 4 + ctime(&info_p->st_atimespec));
    printf_name1(filename, color);
    printf("\n");
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
void printf_name1(char* name, int color) {
    if (color == GREEN) {
        printf("\033[1m\033[32m%s\033[0m", name);
    } else if (color == BLUE) {
        printf("\033[1m\033[34m%s\033[0m", name);
    } else if (color == WHITE) {
        printf("%s", name);
    } else if (color == LBLUE) {
        printf("\033[1m\033[36m%s\033[0m", name);
    } else if (color == YELLOW) {
        printf("\033[1m\033[33m%s\033[0m", name);
    }
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
void do_ls_li(char dirname[]) {
    DIR* dir_ptr;
    struct dirent* direntp;
    if ((dir_ptr = opendir(dirname)) == NULL)
        fprintf(stderr, "lsl:cannot open %s\n", dirname);
    else {
        while ((direntp = readdir(dir_ptr))) {
            restored_ls(direntp);
        }
        sort(filenames, 0, file_cnt - 1);
        int j = 0;
        for (j = 0; j < file_cnt; ++j) {
            if ((Vec & 0b0100010) == 0b0100010) {
                if (filenames[j][0] == '.')
                    continue;
            }
            char temp1[PATH_MAX];
            sprintf(temp1, "%s/%s", dirname, filenames[j]);
            dostat(temp1, filenames[j]);
        }

        closedir(dir_ptr);
    }
}

// void ls_R(char path[]) {
printf("%s:\n", path);
DIR* dir_ptr;
struct dirent* direntp;
if ((dir_ptr = opendir(path)) == NULL)  // 打开目录
    fprintf(stderr, "lsl:cannot open %s\n", path);
else {
    if (direntp->d_name[0] == '.')
        continue;
    while ((direntp = readdir(dir_ptr)) != NULL)  // 读取当前目录文件
    {
        printf("%s  ", direntp->d_name);
    }
}
printf("\n");
closedir(dir_ptr);
if ((dir_ptr = opendir(path)) == NULL)  // 打开目录
    fprintf(stderr, "lsl:cannot open %s\n", path);
else {
    while ((direntp = readdir(dir_ptr)) != NULL) {
        if (direntp->d_name[0] == '.')
            continue;
        struct stat info;
        char temp[PATH_MAX];
        sprintf(temp, "%s/%s", path, direntp->d_name);
        if (lstat(temp, &info) == -1)
            perror(temp);
        if (S_ISDIR(info.st_mode))  // 判断是否为目录，如果是目录就进入递归
        {
            ls_R(temp);
        }
    }
}
