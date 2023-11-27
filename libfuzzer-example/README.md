
## 环境准备

```shell
# 版本不低于6.0
sudo apt install clang

clang -v
clang version 16.0.6 (Red Hat 16.0.6-1.el9)
```

## 直觉

一个完成的程序进行黑盒测试，可能需要考虑从标准输入，还是从文件输入(当然这两者可以互相转换)。

libfuzzer更像单元测试。构建输入。输入来自于种子文件。接口如下。

```c++
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  DoSomethingInterestingWithMyAPI(Data, Size);
  return 0;  // Values other than 0 and -1 are reserved for future use.
}
```

## 参考链接

* https://llvm.org/docs/LibFuzzer.html
* https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md