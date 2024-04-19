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

## 使用 macOS

1. 使用 `brew` 装必备包：

    ```bash
    brew install mc ncurses
    ```

2. 使用 `SOURCE_UTF8_linux_lab_rab` 包中的内容！！！这个包里的代码已经经过了重新编码和优化，支持 64 位环境

3. 将 `Makefile` 中 `gcc -o absloadm.exe` 编译部分改为自己的 `-lncursesw` 环境，以便支持 `UTF-8`

4. 然后执行以下命令进行编译测试
    ```bash
    # 编译工程
    ./GenSysProg
    
    # 执行项目
    ./StartTestTask
    ```

    



## 使用 ubuntu

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

然后将 `SOURCE_KOI8_linux_lab_rab` 中那堆糟糕的文件转换为 `UTF-8` 编码后编译：

- *（使用 `UTF8_linux_lab_rab` 包的话省略这一步）要将源文件从 KOI-8 重新编码为 UNICOD (UTF8)，请执行 shell 脚本 `ChangeCodTable`。*



# 实验工具包文件说明

## 初步说明

> 1. Данный комплект, состоящий из трех элементов учебной системы программирования и тестового задания:
>      - компилятор    PL/1 --> Ассемблер   ( komppl.c ),
>      - компилятор    Ассемблер --> объектный образ  ( kompassr.c ),
>      - абсолютный загрузчик, эмулятор и отладчик ( absloadm.c ),
>      - тестовое задание ( examppl.pli и spismod ).
>
> 2. Для получения исполняемых модулей двух компиляторов и абсолютного загрузчика следует выполнить Bash-скрипт GenSysProgr.
>
> 3. После получения исполняемого кода компиляторов можно запускать на выполнение тестовый пример с помощью Bash-скрипта StartTestTask.

| 文件                       | 说明                       | 功能                                  |
| -------------------------- | -------------------------- | ------------------------------------- |
| `komppl.c`                 | PL/1 编译器                | PL/1 --> 汇编（IBM System/370指令集） |
| `kompassr.c`               | 汇编编译器                 | 汇编 --> 对象                         |
| `absloadm.c`               | 绝对装载器、模拟器和调试器 |                                       |
| `examppl.pli` 和 `spismod` | 测试任务/代码              |                                       |

- 执行 Bash 脚本 `GenSysProgr`，获取**两个编译器**和**绝对加载器**的可执行文件（生成编程系统），该部分涉及从源代码 C 文件 `Komppl.c` 生成可执行文件 `Komppl.exe`。
- 获取编译器的可执行代码后，即可使用 `StartTestTask` Bash 脚本运行测试用例。`StartTestTask` 中有关从 TLD（即 `Komppl.exe` 文件）运行编译器的部分，以编译演示示例（即 `examppl.pli` 文件）。



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



