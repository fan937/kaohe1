通过查询豆包等AI工具搭建Windows下WSL2 Ubuntu22.04开发环境，配置编译工具、Git、Python虚拟环境，使用VSCode远程连接WSL，完成Linux下代码开发环境部署。
问题1：在微软商店下载Ubuntu的版本不对。报错原文：Welcome to Ubuntu 26.04.1 LTS。解决方法：在PowerShell中输入命令wsl --install -d Ubuntu-22.04直接安装。
问题2：在安装vscode的开发插件时显示错误，无法安装。报错原文：无法识别二进制文件。解决方法：使用国外网络进行安装。
问题三：用WSL输入指令验证时远程连接失败。报错原文：/mnt/d/Microsoft VS Code/bin/code: 62: /mnt/d/Microsoft VS Code/Code.exe: Exec format error。解决方法：打开VScode的远程菜单，新开一个VSCode窗口远程连接WSL。
