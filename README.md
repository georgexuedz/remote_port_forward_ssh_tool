ssh远程端口复用工具
=======


jeDate除了包含 日历可以直接显示与点击显示、日期标注点、设定年月（YYYY-MM）、日期范围限制、开始日期设定、自定义日期格式、时间戳转换、当天的前后若干天返回、时分秒选择、智能响应、自动纠错、节日识别，操作等常规功能外，还拥有更多趋近完美的解决方案。您可以免费将她用于任何个人项目。但是不能去除头部信息。 QQ群：516754269 


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
        
***reverse_ssh_info***

        "want_port": 内网连接外网ssh服务，请求分配的端口，这里应该默认为0，让ssh服务自动分配，不需要修改,
        "listen_host": "localhost",
        "device_id": 废弃,
        "port_file_path": "/tmp/",
        "exit_idle_time": 600
        
        
***manager***

        "process_cnt": 1,
        "restart_time": 86400,
        "exit_ssh_time": 1800,
        "lock_file": "/var/run/nc2rctrl.pid"



============
