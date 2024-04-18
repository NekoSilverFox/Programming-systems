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





