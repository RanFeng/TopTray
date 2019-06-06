# TopTray1.0

TopTray 有两个功能:

- 使一个窗口最小化至托盘
- 使一个窗口置顶**并**半透明

上述功能对任何一个有标题栏的窗口有作用。置顶窗口之后，窗口不会有任何提示，所以我加了一个使窗口半透明的功能以提示该窗口已被置顶，如果不需要半透明功能，或者想要自定义透明度，可以修改源码重新编译。

TopTray is free, open source, and is distributed under the terms of the [GNU General Public Licence](http://www.gnu.org/copyleft/gpl.html).

## Download

- [64 bit binaries](x64/)

## Installing

下载之后直接双击即可运行，无需安装。

保证软件完整：

- TopTray.exe
- TopHook.dll

## Using

- 使一个窗口最小化至托盘，用如下任一方法激活此功能：
  - 右键点击标题栏的最小化按钮
  - 按住shift按键再右键点击标题栏
- 使一个窗口置顶**并**半透明，用如下任一方法激活此功能：
  - 右键点击标题栏的最大化按钮
  - 按住ctrl按键再右键点击标题栏
  - 再次操作上述任一步，会恢复窗口至未置顶状态
- 退出。软件运行时无任何提示，退出方法：
  - 命令行运行：TopTray.exe --exit
  - 右键点击被托盘化的程序托盘图标，点击Exit

## Change history

- TopTray1.0 是开源软件 RBTray4.8 更改的，原软件功能是使窗口最小化至托盘，TopTray添加了置顶功能


## Other

For original forum, bug tracker, etc. see [TopTray SourceForge project page](http://sourceforge.net/projects/rbtray/).

Copyright &copy; 1998-2011 Nikolay Redko, J.D. Purcell

Copyright &copy; 2015 Benbuck Nason
