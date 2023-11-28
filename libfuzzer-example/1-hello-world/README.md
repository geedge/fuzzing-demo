
```shell
# 代码覆盖率
clang++ -fsanitize=address,fuzzer -fprofile-instr-generate -fcoverage-mapping test_fuzzer.cc

# 种子库
# ./a.out CORPUS

# 种子可以提供的覆盖率，不包括突变的部分
./a.out CORPUS/*

# 合并代码覆盖率
llvm-profdata merge -sparse *.profraw -o default.profdata

# 远程连接的机器看不到图片
llvm-cov show a.out -instr-profile=default.profdata -name=FuzzMe
```