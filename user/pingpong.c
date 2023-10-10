#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 当你创建一个管道时，你会得到两个新的文件描述符：
// 一个用于管道的读端，另一个用于写端。
#define RD 0
#define WE 1

int main(int argc, char *argv[]) {
  // 实验要求两个管道实现
  int fpipe[2]; // 父进程->子进程
  int cpipe[2]; // 子进程->父进程
  pipe(fpipe);  // 创建管道
  pipe(cpipe);
  int fpid = getpid();
  int cpid = fork();
  if (cpid == 0) { // 子进程
    close(fpipe[WE]);
    close(cpipe[RD]);
    char cbuf; // 存放父进程发送的字节
    // 子进程先读后写,read等待直到可读,这保证了时序关系
    read(fpipe[RD], &cbuf, 1); // 从fpipe读
    fprintf(1, "%d: received ping\n", cpid);
    write(cpipe[WE], "x", 1); // 向cpipe写

    close(fpipe[RD]);
    close(cpipe[WE]);
    exit(0);
  } else { // 父进程
    close(fpipe[RD]);
    close(cpipe[WE]);
    char fbuf;                 // 存放子进程发送的字节
    write(fpipe[WE], "x", 1);  //  向fpipe写
    read(cpipe[RD], &fbuf, 1); // 从cpipe读
    fprintf(1, "%d: received pong\n", fpid);

    close(fpipe[WE]);
    close(cpipe[RD]);
    wait((void *)0); // 等待子进程结束
    exit(0);
  }
}
