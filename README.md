# 开始菜单

## 编译安装
```
# yum install mate-panel-devel cmake gettext libwnck3-devel glib2 gcc-c++ gtkmm30-devel accountsservice-devel 
# mkdir build
# cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
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
3. 在对话框中找到 "Kiran Start Menu", 点击 "添加" 按钮即可
