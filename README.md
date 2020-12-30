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

## 设置
开始菜单、窗口切换插件和工作区预览插件均采用gsettings的方式来提供配置选项

| 插件 | GSettings路径                                                                                       | 说明| since|
| :---- | ------------- | ---- | ----- |
|开始菜单|com.unikylin.Kiran.StartMenu.profile.default-page      |   开始菜单紧凑模式下显示的默认页面, "favorite apps"表示收藏页，"all apps"表示应用列表页| 2.2.0 |
|开始菜单|com.unikylin.Kiran.StartMenu.profile.background-opacity|  开始菜单窗口不透明度，有效范围为0.0 ~ 1.0         | 2.2.0|
|开始菜单|com.unikylin.Kiran.StartMenu.profile.display-mode      |   开始菜单显示模式，compact为紧凑模式，expand为扩展模式| 2.2.0|
|窗口切换预览|com.unikylin.kiran.taskbar.show-active-workspace-apps  |   任务栏上是否只显示当前活动工作区窗口的应用按钮 | 2.2.1  | 
|工作区切换|com.unikylin.kiran.workspace-switcher.draw-windows-in-thumbnails  |   是否在工作区缩略图中绘制窗口缩略图 | 2.2.1  | 
