#include <dirent.h>
#include <stdio.h>
#include <string.h>
void show_ls(char filename[]);
int main(int argc, char* argv[]) {
    // 如果只有一个参数,说明这个命令后面没有加任何参数,也就是./out
    if (argc == 1) {
        show_ls(".");
    }
    // 如果参数的个数不是 1,就循环打印出所有的目录
    while (--argc) {
        // 打印出文件名称
        printf("%s: \n", *++argv);
        show_ls(*argv);
        printf("\n");
    }
    return 0;
}
void show_ls(char filename[]) {
    DIR* dir_ptr;            // the directory
    struct dirent* direntp;  // each entry
    dir_ptr = opendir(filename);
    // dir_ptr == NULL,说明这个目录名称有误
    if (dir_ptr == NULL) {
        fprintf(stderr, "ls1: cannot open%s \n", filename);
        // 而stderr是无缓冲的，会直接输出。
    }
    while ((direntp = readdir(dir_ptr)) != NULL) {
        // readdir()在每次使用后，readdir会读到下一个文件，readdir是依次读出目录中的所有文件，每次只能读一个
        printf("%-10s", direntp->d_name);
    }
    closedir(dir_ptr);  // 关闭流
}