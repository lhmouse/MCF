### 简介
### Introduction

MCF 是一个基于 C++17 的 Windows 框架。  
MCF is a C++17 framework for Windows application development.  

MCF 遵循“无尘设计”的原则。  
其设计目标是将 C 和 C++ 的标准库连带 CRT 全部丢弃，然后重新设计，以期移除任何杂质和智障功能，包括区域和语言设置、标准输入输出流、线程和 thread_local 等。  
MCF is a clean room design of C++.  
That is, the goal of MCF is to destruct the C and C++ standard libraries as well as the CRT, then rebuild a subset of them, effectively removing mistaken features such as locales, iostreams, threads and thread_local, etc.  

### 如何构建
### How to Build

你必须使用这个工具链构建 MCF，因为其依赖 GCC 的 C++ 库，而不同线程模型并不兼容： <https://gcc-mcf.lhmouse.com/>  
而 MCFCRT 不依赖 GCC 的线程模型，所以使用任何目标为 \*-w64-mingw32 发行版的 GCC 都无所谓。  
You must use these toolchains to build MCF because of incompatibility of GCC's thread models used to build GCC C++ libraries: <https://gcc-mcf.lhmouse.com/>  
The library MCFCRT does not rely on GCC's thread models thus can be built with any GCC distribution targeting \*-w64-mingw32.  

### GIT SUCKS

@FrankHB recommended this article https://gist.github.com/incompl/3819571 that I'd like to share with you.  
Because that is exactly what I want to say and I am simply too busy to write one.  

lh_mouse  
2015-12-07  

### IRC channel:

<https://webchat.freenode.net/?channels=%23mcfproj>

### 问题反馈
### Bug Reports

请联系 lh_mouse at 126 dot com（注明 MCF 相关）。  
Please email to lh_mouse at 126 dot com. (Please state your email as related to MCF.)  
