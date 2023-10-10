#include "kernel/types.h"
#include "user/user.h"

#define RFD 0
#define WFD 1

// 从管道中取出第一个数字
// 成功返回0,否则(为空)返回-1
int getFirstNum(int lPipe[], int *dst);

// 除了第一个进程和最后一个进程
// 其他进程的行为都是相似的
__attribute__((noreturn)) void primes(int lPipe[]);

// 将lPipe中符合要求的数字送入rPipe中
// 要求:不能是第一个数的倍数
void transmitData(int lPipe[], int rPipe[], int firstNum);

void transmitData(int lPipe[], int rPipe[], int firstNum) {
  int buf;
  while (read(lPipe[RFD], (void *)&buf, sizeof(int)) == sizeof(int)) {
    if (buf % firstNum != 0) {
      write(rPipe[WFD], (void *)&buf, sizeof(int));
    }
  }
}

int getFirstNum(int lPipe[], int *dst) {
  if (read(lPipe[RFD], (void *)dst, sizeof(int)) == sizeof(int)) {
    return 0;
  } else {
    return -1;
  }
}

void primes(int lPipe[]) {
  // 当前进程拿到一对文件描述符,指向一个管道
  // 这个管道按照升序存放着一组数字
  //
  // 取出第一个数字,打印
  // 后面的数字
  //      1. 是第一个数字的倍数:丢弃
  //      2. 否则:放入新的管道,交给子进程处理
  //
  // 对lPipe只读不写,关闭写
  close(lPipe[WFD]);
  // 从管道中捞出第一个数来
  // 这个数是素数，后面数字若不能被整除，则放入rPipe
  int firstNum;
  if (getFirstNum(lPipe, &firstNum) == 0) {
    // 左边的管道中还有数据
    printf("prime %d\n", firstNum);
    int rPipe[2];
    pipe(rPipe);
    // 对于新的rPipe当前进程只写不读
    transmitData(lPipe, rPipe, firstNum);
    // rPipe中已经放进了数据，也有可能已经没有数据了
    // 将lPipe中的数据读出写入rPipe后,当前进程不再读写lPipe
    // 所以关闭lPipe的读
    // 当前进程也不再读写rPipe,但是不能关闭rPipe的读和写
    // 因为要fork后子进程会获得父进程打开文件描述符的副本
    // 若fork前关闭rPipe的读写,子进程也无法读写rPipe
    close(lPipe[RFD]);
    if (fork() == 0) {
      primes(rPipe);
    } else {
      close(rPipe[RFD]);
      close(rPipe[WFD]);
      wait(0);
      exit(0);
    }
  } else { // 左边给的管道中已经没有数据了
    close(lPipe[RFD]);
    exit(0);
  }
}

int main() {
  int p[2];
  pipe(p);
  for (int i = 2; i <= 35; i++) {
    write(p[WFD], (void *)&i, sizeof(int));
  }
  if (fork() == 0) {
    primes(p);
  } else {
    close(p[WFD]);
    close(p[RFD]);
    wait(0);
  }
  exit(0);
}
