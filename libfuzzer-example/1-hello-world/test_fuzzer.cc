#include <stddef.h>
#include <stdint.h>

// 编译和链接的时候都进行插桩;内存错误检查
// clang++ -fsanitize=address,fuzzer test_fuzzer.cc

// 默认使用一半的处理器;最大长度没有设置，默认不超过4096;没有提供预料库
// ./a.out

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size > 0 && data[0] == 'H')
    if (size > 1 && data[1] == 'I')
      if (size > 2 && data[2] == '!')
        // 编译器提供的内建函数:通常用于标识代码中的不可达分支或者表示一个程序错误的严重情况。不执行任何清理工作，是一种程序停止的强烈信号
        __builtin_trap();
  return 0;
}