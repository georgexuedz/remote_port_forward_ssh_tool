# Tutorial

![架构图](/pic/frame_pic.png "架构图")

#1.编译：  
>	1) 编译环境   
>>	  	1.`安装libssh2`
>>		2.安装cJSON  
	  
>	2) 编译选项：
>>		1.make clean 删除编译中间文件
>>		2.make ver=debug 生成有调试信息的程序
>>		3.make ver=release 生成正式程序
	
>	3) 编译结果：nc2rctrl   
#2.运行   
>	nc2rctrl -f nc2rctrl.conf   
#3.路径：
>	1) 运行环境：
>>		a. 单例辅助文件
>>>			/var/run/nc2rctrl.pid   

>	2) 远程环境：
>>		a. 登录端口文件
>>>>			A.路径
>>>>>				由nc2rctrl.conf的"port_file_path"指定
>
>>>>			B.格式
>>>>>				"port_file_path"/MAC
>
>>>>			C.内容
>>>>>				端口数字

##<a name="table"/>device
字段  | 意义
------------- | -------------
username  | 运行环境登录远程环境的ssh用户
password  | 运行环境登录远程环境的ssh密码
bindhost  | 运行环境登录远程环境的ssh密码
ssh_listen_port |  运行环境登录远程环境的ssh密码
