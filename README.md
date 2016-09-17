ssh远程端口复用工具
=======


该工具，主要解决维护内网服务器。举例的场景就是内网的路由器，wifi或者防火墙等网络设备。  
原理就是ssh的远程反向隧道，让内网的ssh客户端连接代理服务器的ssh服务，建立连接，然后用户就可以登录代理服务器，使用建立的连接来ssh登录
内网的ssh服务。  
完善的多种保护机制，让该工具的稳定性大大提高，基于C语言，执行效率高。  
主要是使用了libssh2这个C语言库，文档较少，遇到问题比较难求助，因此，只能自力更生，看源码啦！看源码的时候需要和ssh的协议对照看，才能
了解大部分的逻辑。 
由于该工具，基于C语言，因此很多还需要完善，希望大家一起完善哈！QQ:845094708, george   
 

* [下载设计文档](https://github.com/georgexuedz/remote_port_forward_ssh_tool/blob/master/doc/requirement_analysis/solution_choosing.docx)
* [下载使用文档](https://github.com/georgexuedz/remote_port_forward_ssh_tool/blob/master/doc/guide/user_guide.docx)
* [下载libssh2源码分析文档](https://github.com/georgexuedz/remote_port_forward_ssh_tool/blob/master/doc/ssh_research/libssh2_code_analysis.doc)


![image](https://github.com/georgexuedz/remote_port_forward_ssh_tool/raw/master/doc/image/introduction.jpg)


# 快速上手

**编译**

    安装libssh2
    进入目录，执行：git submodule update --init --recursive
    
    make ver=debug 生成有调试信息的程序
    make ver=release 生成正式程序
    make clean 删除编译中间文件
      
      
**配置 (src/conf/nc2rctrl.conf)**

***device 运行该工具的环境***

        "username": 内网ssh账号,
        "password"：内网ssh密码,
        "bind_host": 内网的ssh服务地址,
        "ssh_listen_port": 22内网ssh服务监听端口,
        
***agent 代理服务器***

        "username": 外网ssh账号,
        "password": 外网ssh密码,
        "bind_host": 外网ssh服务的地址,
        "ssh_listen_port": 外网ssh服务监听端口,
        
***reverse_ssh_info 远程ssh反向代理信息***

        "want_port": 内网连接外网ssh服务，请求分配的端口，这里应该默认为0，让ssh服务自动分配，不需要修改,
        "listen_host": 外网代理服务器访问ssh服务的host，如果为同一个设备，则"localhost",
        "port_file_path": 代理服务器保持ssh连接的文件路径,文件名用mac地址命名,内容是端口号,
        "exit_idle_time": 断开连接超时,
        
        
***manager 管理进程***

        "process_cnt": 开启连接进程数量,
        "restart_time": 数据进程重启超时时间（秒）,防止异常导致一直不能重启,
        "exit_ssh_time": 无数据传输超时重启时间（秒）,
        "lock_file": 单例加锁文件

**运行**


***代理服务器***

    目录 /data/nc2rctrl, 添加读写权限，存储连接端口文件, 格式例子： /data/nc2rctrl/00-0C-29-66-1E-2A

***内网服务器***



    /usr/sbin/nc2rctrl -f nc2ctrl.conf


# 保护机制
    1. 如果连接之后，经过半小时，管理进程就会发送信号sigint给隧道进程，如果接收到信号，并且还没登录，隧道就退出重启；
        或者登陆后10分钟没有操作且没有数据传递，隧道就断开重启；
    2. 一天之后，管理进程重启，杀死所有隧道，全部重启，防止资源泄漏；
    3. cron每天，尝试启动一次管理进程，如果已经有管理进程在运行，就退出；
    
    
    
    
    
