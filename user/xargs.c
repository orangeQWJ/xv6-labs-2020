#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

// echo "1\n2" | xargs  echo line
// 首先通过argv获取
//		echo 要执行的程序
//		"line" echo 的参数1
//	通过 被重定向过的stdin分两次读取 “1”  “2”  作为echo的参数
//
// 最终构造出
//		echo line 1
//		echo line 2
// 创建新进程指令上面两个进程，并等待子进程返回

int main(int argc, char *argv[]) {
  // 存放诸如 “1” “2” 这样的参数,这些参数从stdin读入
  char extern_arg[512];
  // 存放xargs 自身所有参数 eg: echo line
  char *argv_std[MAXARG];

  // 将 xargs echo line 存入argv_std
  for (int i = 1; i < argc; i++) {
    argv_std[i - 1] = argv[i];
  }

  // 暂存从stdin读入的字符
  char chat_buf;
  // 指向存放下一个字符的位置
  char *p = extern_arg;

  while (read(0, (void *)&chat_buf, 1) == 1) {
    if (chat_buf != '\n') {
      *p = chat_buf;
      p++;
    } else {
      // 以0结尾使字符数组变成字符串
      *p = 0;
      // 回到开始位置，等待下一次存储
      p = extern_arg;
      // 将stdin读入的参数加到参数数组后面
      argv_std[argc - 1] = extern_arg;
      // 为参数数组添加结尾
      argv_std[argc] = 0;

      // 创建子进程
      if (fork() == 0) {
        exec(argv_std[0], argv_std);
      } else {
        wait(0);
      }
    }
  }
  exit(0);
}
