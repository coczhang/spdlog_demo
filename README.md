# spdlog是一个开源、跨平台、无依赖、只有头文件的C++11日志库

# 源码下载地址
spd源码:https://github.com/gabime/spdlog
cmake gui:https://cmake.org/download/

# 配置
1.解压spdlog源码，并打开解压后的文件夹，新建build文件夹  
2.打开cmake，添加源码目录和输出目录，见下图：  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_01.png)  
3.点击cmake UI的configure，下面选择的是win32，点击finish  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_02.png)  
4.configure 完成后， 选择合适的工程， 再点击 generate生成对应的 工程文件。  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_03.png)  
5.生成成功， 至此， 配置到此结束，  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_04.png)  

# 方案
1.打开 cmake的输出目录（我这里是在源码解压目录下build文件夹），（注意：请使用管理员方式打开VS201X ? 因为要将文件安装到指定的系统文件夹下，没有对应的权限，程序无法执行写入）打开解决方案 spdlog.sln （以实际生成的为准）：  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_05.png)  
2.选择 项目：ALL_BUILD, 右键选择生成。  
3.待上一步生成结束后， 选择 项目： INSTALL， 右键生成。 可以观察到输出窗口中 将 spdlog文件安装到系统的具体目录。  
![image] (https://github.com/coczhang/spdlog_demo/blob/main/image/spd_06.png)  
安装结束后可以阅读源码目录下的example文件夹下的example.cpp, 总结其使用方法。