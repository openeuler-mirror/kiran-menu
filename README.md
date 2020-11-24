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
  * accountsservice-devel
  * gtk-update-icon-cache

## 编译安装
```
   # yum install mate-panel-devel cmake gettext libwnck3-devel glib2 gcc-c++ gtkmm30-devel cairomm-devel accountsservice-devel mate-desktop-devel gtk-update-icon-cache
   # mkdir build
   # cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
   # make
   # sudo make install
   # sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
   # sudo gtk-update-icon-cache -f /usr/share/icons/hicolor/
```

## 运行测试
```
    # cd build && ./test/start-menu/test-start-menu
```

## 运行
1. 安装运行依赖

```
    # yum install mate-screensaver accountsservice mate-session-manager mate-control-center caja yelp
```
2. 右击面板，选择"添加到面板"
3. 在对话框中找到 "Kiran开始菜单"，"Kiran应用窗口切换插件"或"Kiran工作区切换插件”, 点击 "添加" 按钮即可
