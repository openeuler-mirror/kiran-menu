# kiran-menu

## Build Environment Requirements

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

## Build and Installation

```bash
   # mkdir build
   # cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr .. 
   # make
   # sudo make install
   # sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
   # sudo gtk-update-icon-cache -f /usr/share/icons/hicolor/
```

## Execution

1. Install the following runtime dependencies:
    * mate-screensaver
    * kiran-session-daemon
    * mate-session-manager
    * mate-control-center
    * caja
    * yelp
2. Right-click the panel and select **Add to Panel**.
3. Locate **Kiran Start Menu**, **Kiran Taskbar Window Switcher**, or **Kiran Workspace Switcher** in the dialog box, and click **Add**.

## Debugging

The application leverages the `zlog` framework for logging. By default, it reads the configuration from `/etc/zlog.conf` and outputs log messages to `$HOME/.cache/kiran/kiran-applet/kiran-applet.log`.

For debugging purposes, you can override the default configuration path by setting the `ZLOG_CONF` environment variable.

## Settings

The **Start Menu**, **Taskbar Window Switcher**, and **Workspace Preview** plugins all leverage `GSettings` to manage their configuration options.

| Plugin | GSettings Path | Description | Since |
| :--- | :--- | :--- | :--- |
| **Start Menu** | `com.kylinsec.kiran.startmenu.profile.default-page` | The default page displayed when **Start Menu** is in compact mode. `favorite apps` represents the favorites page, and `all apps` represents the application list page. | 2.2.0 |
| **Start Menu** | `com.kylinsec.kiran.startmenu.profile.background-opacity` | The opacity of the **Start Menu** window. Valid range is `0.0` to `1.0`. | 2.2.0 |
| **Start Menu** | `com.kylinsec.kiran.startmenu.profile.display-mode` | The display mode of **Start Menu**. Options are `compact` (Compact Mode) or `expand` (Expanded Mode). | 2.2.0 |
| **Taskbar Window Switcher** | `com.kylinsec.kiran.taskbar.show-active-workspace-apps` | Specifies whether the taskbar should only display application buttons for windows on the currently active workspace. | 2.2.1 |
| **Workspace Switcher** | `com.kylinsec.kiran.workspace-switcher.draw-windows-in-thumbnails` | Specifies whether to render window thumbnails inside the workspace thumbnails. | 2.2.1 |
