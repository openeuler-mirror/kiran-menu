# 开始菜单

## 编译环境要求
  * mate-panel-devel >= 1.22
  * mate-desktop-devel >= 1.22
  * cmake >= 3.0
  * gettext
  * libwnck3-devel
  * glib2
  * gcc-c++ >= 4.8
  * gtk3-devel >= 3.22
  * gtkmm30-devel
  * cairomm-devel
  * libX11-devel
  * libxcb-devel
  * gtk-update-icon-cache
  * accountsservice-devel

## 编译安装

```
   # mkdir build
   # cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr .. -DBUILD_WITH_ACCOUNTSSERVICE=on
   # make
   # sudo make install
   # sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
   # sudo gtk-update-icon-cache -f /usr/share/icons/hicolor/
```
> 插件默认依赖`accountsservice`来提供当前用户的账户信息。如果需要用`kiran-session-daemon`来替代`accountsservice`来提供账户信息，请安装 `kiran-cc-daemon-devel`，并将编译指令中的`BUILD_WITH_ACCOUNTSSERVICE`参数替换为`BUILD_WITH_KIRANACCOUNTS`

## 运行
1. 安装以下运行依赖:
 * mate-screensaver
 * accountsservice 或 kiran-session-daemon
 * mate-session-manager
 * mate-control-center
 * caja
 * yelp
2. 右击面板，选择"添加到面板"
3. 在对话框中找到 "Kiran开始菜单"，"Kiran应用窗口切换插件"或"Kiran工作区切换插件”, 点击 "添加" 按钮即可

## 调试
程序采用zlog接口进行日志记录。默认情况下，程序会读取/etc/zlog.conf配置，并将日志信息会打印到$HOME/.cache/kiran/kiran-applet/kiran-applet.log中。

调试时可以通过设置ZLOG_CONF环境变量来自定义zlog配置文件路径。
