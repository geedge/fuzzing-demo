
## 环境准备

```shell
# 版本不低于6.0
sudo apt install clang # 编译C/C++代码需要
sudo dnf install llvm # 获取覆盖率的时候需要

clang -v
clang version 16.0.6 (Red Hat 16.0.6-1.el9)
```

## 基本使用

### 简单的target

一个完整的程序进行黑盒测试，可能需要考虑从标准输入，还是从文件输入(当然这两者可以互相转换)。

libfuzzer更像单元测试。构建输入。输入来自于种子文件。接口如下。

但是模糊测试的粒度，相较于单元测试，必然更粗些。应该按照场景去组合使用库提供的API。

```c++
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  DoSomethingInterestingWithMyAPI(Data, Size);
  return 0;  // Values other than 0 and -1 are reserved for future use.
}
```

编译。

```shell
# 编译和链接的时候都进行插桩;内存错误检查
clang++ -fsanitize=address,fuzzer test_fuzzer.cc
```

### 其他

1. 语料库。 
   * 输入需要特定结构的模糊测试，直接生成输入是低效的，需要提供(优质)种子库。
   * 最小化语料库。(将语料库合并，不能提供新的路径的种子丢弃。-merge选项控制)
   * fork模式：进程本身不会进行任何模糊处理，但会产生多达 N 个并发子进程，为它们提供语料库的随机子集。在孩子退出后，top进程将孩子生成的语料库合并回主语料库。（基本上，-fork=N 等效为-jobs=N and -workers=N）
   * ...
   * 字典。如果被模糊化的输入格式由标记组成或具有大量魔法值，那么这种方法很有效。(单纯的模糊测试，突变的时候是不进行语法分析的。) (这个字典是咋创建的？更具语法，将拆出每个关键字？根据字典，模糊测试器，有一定几率生成合法的种子，应该是这样)
   * 直接使用崩溃输入可以复现崩溃。也可以突变崩溃文件。很少用，感觉。
2. 突变。
   * 突变策略。非模糊器的开发者，通常不需要考虑。
3. 加速测试与资源限制。
   * 默认是单线程进行测试(除非被测试的库启动自己的线程)。`-jobs`参数可以控制，测试使用的(处理器)数量。(-job=30。但是当前cpu只有12个，默认只能分配一半(-work=min(jobs, NumberOfCpuCores()/2))，即6个cpu。所以同时运行的这6个测试进程，平均每个需要发现5个bug(进程因为bug停止后，会重新再创建一个)，程序才会停止。这也是为什么，不使用job参数的时候，遇到一个bug程序即停止) （这里的并行测试会共享语料库。但应该共享不了路径吧。）
   * -runs 限制尝试次数。
   * 可以拒绝不需要的输入。比如，只希望解析的输入进入语料库。
4. 暂时忽略部分。
     * 代码覆盖率。使用clang coverage。(先测试，然后拿个原来的种子和生成的种子重新跑一遍，得到覆盖率) （生成的覆盖率文件llvm-cov show，远程连接看不到。后面尝试尝试转成图片格式查看吧。）
     * 与其他模糊测试器的兼容。
     * ...
## 参考链接

* https://llvm.org/docs/LibFuzzer.html
* https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md