<!-- SPbSTU 报告起始 -->

<div align="center">
  <!--<img width="250px" src="https://raw.githubusercontent.com/NekoSilverFox/NekoSilverfox/403ab045b7d9adeaaf8186c451af7243f5d8f46d/icons/new_logo_spbstu_ru.svg" align="center" alt="new_logo_spbstu_ru" />  新式 π logo -->
  <img width="250px" src="https://github.com/NekoSilverFox/NekoSilverfox/blob/master/icons/logo_building_spbstu.png?raw=true" align="center" alt="logo_building_spbstu" /> <!-- 研究型大学 logo -->
  </br>
  <b><font size=3>Санкт-Петербургский политехнический университет Петра Великого</font></b>
  </br>
  <b><font size=2>Институт компьютерных наук и технологий</font></b>
  </br>
  <b><font size=2>Высшая школа программной инженерии</font></b>
</div>


<div align="center">
<b><font size=6>Системы программирования</font></b>



[![License](https://img.shields.io/badge/license-Apache%202.0-brightgreen)](LICENSE)
![image](https://img.shields.io/badge/Docker-i386--ubuntu-orange)

</div>
<div align=left>
<div STYLE="page-break-after: always;"></div>
<!-- SPbSTU 报告结束 -->


[toc]



# 环境配置

## 基于 docker

尽量使用 `x86` 架构下进行实验，否则会造成调试上的很多麻烦

> 基于 M-Chip Mac

1. 安装 Docker

    确保你的 M1 MacBook 上安装了最新版本的 Docker Desktop for Mac（Apple Silicon 版本）。这可以从 Docker 的官方网站直接下载安装。

2. 启用模拟支持
    Docker Desktop for Mac 支持使用 `QEMU` 模拟不同的架构。你需要确保 Docker 的设置中启用了这项功能。一般来说，这个功能默认是开启的。

3. 使用 i386/ubuntu 镜像

    为了在 M1 MacBook 上运行 Ubuntu 的 32 位版本，你可以使用 `i386/ubuntu` 镜像，这是专为 32 位环境准备的。你可以通过以下命令来拉取这个镜像：

    ```
    docker pull i386/ubuntu
    ```

4. 运行 Docker 容器

5. 拉取镜像后，你可以使用以下命令来启动一个基于 i386/ubuntu 镜像的容器：

    ```
    docker run -it --name=ProgSysx86 i386/ubuntu /bin/bash
    ```

    这个命令将启动一个 Ubuntu 容器，并且提供一个交互式的 Bash 会话，允许你在容器内执行命令。

**注意事项：**

- **性能问题**：使用 QEMU 模拟 x86 架构在 ARM 架构上运行时可能会有性能损失。这是因为模拟需要在运行时翻译指令，这会增加额外的计算负担。
- **软件兼容性**：一些软件可能在通过模拟在 ARM 上运行时遇到兼容性问题，特别是那些对性能有高要求或直接与硬件交互的程序。

---

## 安装必备包

> 参考文档：`Рекомендация_3_Низкоуровневый_эквивалент_готов_Что_дальше`

启动 Docker 镜像后，需要安装包：

如果不想看下面这 4 步就直接执行命令：

```bash
apt-get update && apt-get install -y mc libncurses5-dev make gcc vim
```

1. 使用命令更新 apt：`apt-get update`
2. 使用 `sudo apt-get install mc` 指令添加 Midnight Comander (mc) 面板文件管理器软件包
3. 使用 `sudo apt-get install libncurses5-dev` 指令添加 ncurses 伪图形窗口软件包
4. 安装 `make` 和 `gcc`：`apt-get install make gcc`

---

然后将 `linux_lab_rab` 中那堆糟糕的文件转换为 `UTF-8` 编码后编译：

- *（使用 `UTF8_linux_lab_rab` 包的话省略这一步）要将源文件从 KOI-8 重新编码为 UNICOD (UTF8)，请执行 shell 脚本 `ChangeCodTable`。*
- 执行 shell 脚本 `GenSysProg`（生成编程系统）部分，该部分涉及从源代码 C 文件 `Komppl.c` 生成可执行文件 `Komppl.exe`。
- 执行 `StartTestTask` shell 脚本中有关从 TLD（即 `Komppl.exe` 文件）运行编译器的部分，以编译演示示例（即 `examppl.pli` 文件）。
- 验证编译器是否成功编译了演示示例 `examppl.ass` 的等效汇编文件，以及该汇编文件是否包含以下汇编文本：

| Метка | КОП   | Операнды | Пояснения                    |
| ----- | ----- | -------- | ---------------------------- |
| ex    | START | 0        | cчетчик отн. адреса в  нуль  |
|       | BALR  | RBASE,0  | отн. адр. базы в RBASE       |
|       | USING | *,RBASE  | Объявл. RBASE регистром базы |
|       | L     | RRAB,A   | Загрузка A в регистр RRAB    |
|       | A     | RRAB,B   | A+B грузим в RRAB            |
|       | S     | RRAB,C   | A+B-C грузим в RRAB          |
|       | ST    | RRAB,D   | A+B-C  из RRAB грузим в D    |
|       | BCR   | 15,RVIX  | Переход по адр. в  рег. RVIX |
| A     | DC    | F’3’     | A=3                          |
| B     | DC    | F’4’     | B=4                          |
| C     | DC    | F’5’     | C=5                          |
| D     | DS    | F        | Резерв памяти для D          |
| RBASE | EQU   | 5        | RBASE назначим 5             |
| RRAB  | EQU   | 3        | RRAB назначим 3              |
| RVIX  | EQU   | 14       | RVIX назначим 14             |
|       | END   |          | Конец текста блока           |

# linux_lab_rab 中的糟心玩意

## absloadm.c

这段代码是一个仿真器，它模拟了一台IBM System/370（或类似架构）计算机的指令集的子集。这个仿真器使用了ncurses库来创建文本用户界面，允许用户观察和交互地跟踪程序的执行。程序主要分为几个部分：

1. **数据定义**：程序定义了多个数据结构和变量来存储模拟器的状态，如寄存器内容、内存区域、机器指令以及屏幕窗口。
2. **ncurses界面设置**：使用ncurses库创建和管理多个文本窗口，每个窗口显示不同的程序信息，如寄存器状态、内存内容等。
3. **指令处理**：为IBM System/370架构定义的指令实现了解析和执行的逻辑。这包括了操作码（opcode）和寄存器间的操作，如`BALR`和`BCR`。
4. **主循环**：程序中的`sys`函数包含了主循环，它处理指令的执行，更新寄存器和内存的状态，并在ncurses界面上更新显示。
5. **错误处理**：程序在执行过程中可能会遇到各种错误，这些错误在代码的末尾有详细的错误处理和消息输出。
6. **指令集**：程序实现了IBM System/370指令集的一部分，包括`BALR`、`BCR`、`ST`、`L`、`A`和`S`指令。这些指令涵盖了跳转、数据存储、加载和算术操作。
7. **程序启动与初始化**：`main`函数处理命令行参数，进行初步的文件处理，并调用`InitCurses`和`sys`函数来启动模拟器。

这个模拟器的目标是加载一个或多个编译后的模块（通常为二进制格式），并模拟它们的执行。模拟器使用了IBM System/370架构的汇编语言编写，并能够解释和执行一系列定义好的机器指令。程序还包含了一些基本的界面管理函数，用于在命令行界面显示和管理输出。



