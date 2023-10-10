#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

void myFind(char *path, char *name);

int main(int argc, char *argv[]) {
  myFind(argv[1], argv[2]);
  exit(0);
}

void myFind(char *path, char *name) {
  // path 当前所在位置 一个不以 /结尾的路径
  // buf 记录现在所处的位置
  // p 指引着下一个放入buf的字符的起始位置
  char buf[512], *p;
  int fd;
  // Directory is a file containing a sequence of dirent structures.
  // 目录就是一个包含一系列dirent结构的文件
  struct dirent de;
  // 包含文件名

  // 文件元信息,包含文件类型
  struct stat st;

  // 打开目录,fd是目录文件的文件描述符
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  // 通过文件描述符获取目录的元信息
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
  // 判断路径类型,第一个参数必须是目录
  if (st.type != T_DIR) {
    printf("<usage> find 起始目录 要搜索的内容");
    return;
  }
  // 将当前位置存放入buf中
  strcpy(buf, path);
  p = buf + strlen(path);
  *p++ = '/';

  // 将文件夹中所有的文件条目（dirent结构）都读出来
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    // 无效？
    if (de.inum == 0)
      continue;
    if (strcmp(de.name, ".") == 0) {
      continue;
    }
    if (strcmp(de.name, "..") == 0) {
      continue;
    }

    // 首先要打开文件才能读取元信息,
    // 通过dirent 可以知道当前文件/文件夹的名字
    // 但是只有获取元信息后,才知道当前文件是文件夹还是文件
    // 根据元信息,如果类型是文件就判断是不是要找的
    //            如果类型是文件夹就递归查找
    memmove(p, de.name, DIRSIZ);
    // 下面可有可无,de.name 字段已经以\0结尾
    // de.name整个字段长度DIRSIZ
    //*(p + strlen(de.name)) = '\0';

    int tempFd;
    if ((tempFd = open(buf, 0)) < 0) {
      fprintf(2, "find: cannot open %s\n", path);
      return;
    }

    if (fstat(tempFd, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", path);
      return;
    }
    switch (st.type) {
    case T_FILE:
      // 判断是否是我们要的文件
      if (strcmp(de.name, name) == 0) {
        // 将路径连带文件名一起打印出来
        printf("%s\n", buf);
      }
      close(tempFd);
      continue;
    case T_DIR:
      close(tempFd);
      // 将当前条目的名字加到buf中
      myFind(buf, name);
      //break;
    }
  }
  close(fd);
}
