# ETX_AODV
这个项目是基于文章 "Implementation of ETX Metric within the AODV Protocol in the NS-3 Simulator" 中所提出的ETX度量的AODV路由协议进行复现的
用于人工智能课程期末作业-复现文章实验
## 使用
本课题在NS3模拟器中实现AODV路由协议中的ETX度量
此项目的代码需要运行在ns3.28环境下，请保证ns3的版本在此之上
如果你想要运行这些代码，请完成以下工作：
1) 删除/ns-3.28/src/aodv/model下所有的文件
2) 复制aodv/model中的所有文件到/ns-3.28/src/aodv/model中
3) 对于在/ns-3.28/src/aodv文件夹下的wsrcipt脚本也这样操作
5) 写一个使用了本ETX-aodv路由的含有主函数相关逻辑的.cc文件，并放到/ns-3.28/scratch文件夹之中（你也可以直接将我提供的11.cc文件复制到这个位置上）
6) 确定ns3环境无误后，使用命令./waf 11开始运行
   
复现刊物的文章链接:
Telfor Journal, http://journal.telfor.rs/Published/Vol10No1/Vol10No1_A4.pdf
参考论文主页链接：https://ieeexplore.ieee.org/document/8249315
