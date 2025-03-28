# 应用截图
<img width="1080" alt="d" src="https://github.com/user-attachments/assets/14200e57-a6ec-4fb5-bf48-0661cac9a609" />
<img width="1080" alt="A" src="https://github.com/user-attachments/assets/3fbff431-6b35-4dd1-afe1-26741f7d85be" />
<img width="1080" alt="B" src="https://github.com/user-attachments/assets/03acea3a-fb35-40ad-ae20-f79eb2e28c73" />
<img width="1080" alt="C" src="https://github.com/user-attachments/assets/9eb7bf39-05c8-423c-8bb4-70997a236fa7" />


# 编译说明

## 自行搜索教程安装WSL

## 配置环境
### update
    sudo apt update

### 安装gcc，python
    sudo apt install gcc
    sudo apt install python3

### 创建python软链接
    sudo rm -rf /usr/bin/python
    sudo ln -s /usr/bin/python3 /usr/bin/python
### 安装NASM
    sudo apt install nasm

### 安装asl
    sudo apt install iasl

### 安装uuid库
    sudo apt install uuid-dev

### 构建必备软件包的信息列表
    sudo apt install build-essential

### 安装make
    sudo apt install make

## 编译代码

### source
    source 通用固件.sh

### 编译
    build

## 生成EFI

### 安装pip
    sudo apt install python3-pip

### wsl安装若遇到“error: externally-managed-environment”，请运行
    pip config set global.break-system-packages true

### pip 换源
    cd ~                    
    mkdir .pip             # 新建.pip隐藏文件夹
    cd .pip                # 进入.pip文件夹   
    touch pip.conf         # 新建pip.conf文件
    vim pip.conf           # 用vim编辑pip.conf文件

### pip.conf文件内容为：

    [global]
     
    index-url=https://pypi.tuna.tsinghua.edu.cn/simple
     
    timeout = 6000
     
    [install]
     
    trusted-host=pypi.tuna.tsinghua.edu.cn
     
    disable-pip-version-check = true

### cd 返回工作目录"TianJIBoot"

### 然后运行
    pip install -r requirements.txt

### 再运行
    python 生成EFI.py

### 输入屏幕像素宽度和屏幕像素高度即可

## 调整OpenCore所在的EFI分区大小至1GB
自行搜索如何使用Disk genius调整分区大小

## 复制生成的EFI文件至OpenCore（MacOS）所在的EFI分区
若没有OpenCore，选择默认EFI分区即可
<img width="910" alt="1" src="https://github.com/user-attachments/assets/c4507491-f443-476d-80d3-0157219e7d8c" />

## 设置UEFI启动项并将其上移至最顶部
在Disk genius工具栏的“工具”目录下
<img width="912" alt="设置UEFI启动项" src="https://github.com/user-attachments/assets/d3ec0929-be60-43bd-a4fd-06d64f1ced65" />
<img width="518" alt="选择所有文件" src="https://github.com/user-attachments/assets/bf3f9c58-f7ca-44c2-bbca-30f9e4d9c334" />
<img width="581" alt="设置UEFI选项" src="https://github.com/user-attachments/assets/5ea4ca34-941f-45e9-88a4-4a3872e29bd7" />

# Release说明
<img width="1080" alt="a" src="https://github.com/user-attachments/assets/0e830d53-3b53-4743-89fe-bb8f3dfa071e" />
下载source code即可

## 生成EFI

### 安装pip
    sudo apt install python3-pip

### wsl安装若遇到“error: externally-managed-environment”，请运行
    pip config set global.break-system-packages true

### pip 换源
    cd ~                    
    mkdir .pip             # 新建.pip隐藏文件夹
    cd .pip                # 进入.pip文件夹   
    touch pip.conf         # 新建pip.conf文件
    vim pip.conf           # 用vim编辑pip.conf文件

### pip.conf文件内容为：

    [global]
     
    index-url=https://pypi.tuna.tsinghua.edu.cn/simple
     
    timeout = 6000
     
    [install]
     
    trusted-host=pypi.tuna.tsinghua.edu.cn
     
    disable-pip-version-check = true

### cd 返回工作目录

### 然后运行
    pip install -r requirements.txt

### 再运行
    python 生成EFI.py

### 输入屏幕像素宽度和屏幕像素高度即可

## 调整OpenCore所在的EFI分区大小至1GB
自行搜索如何使用Disk genius调整分区大小

## 复制生成的EFI文件至OpenCore（MacOS）所在的EFI分区
若没有OpenCore，选择默认EFI分区即可
<img width="910" alt="1" src="https://github.com/user-attachments/assets/c4507491-f443-476d-80d3-0157219e7d8c" />

## 设置UEFI启动项并将其上移至最顶部
在Disk genius工具栏的“工具”目录下
<img width="912" alt="设置UEFI启动项" src="https://github.com/user-attachments/assets/d3ec0929-be60-43bd-a4fd-06d64f1ced65" />
<img width="518" alt="选择所有文件" src="https://github.com/user-attachments/assets/bf3f9c58-f7ca-44c2-bbca-30f9e4d9c334" />
<img width="581" alt="设置UEFI选项" src="https://github.com/user-attachments/assets/5ea4ca34-941f-45e9-88a4-4a3872e29bd7" />
