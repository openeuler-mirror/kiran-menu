/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "kiran-tray.h"
#include <gtk/gtk.h>

#define CALENDA_WIDTH 1
#define CALENDA_HEIGHT 40

static void tran_setup(GtkWidget *window)
{
    GdkScreen *screen;
    GdkVisual *visual;

    gtk_widget_set_app_paintable(window, TRUE);
    screen = gdk_screen_get_default();
    visual = gdk_screen_get_rgba_visual(screen);

    if (visual != NULL && gdk_screen_is_composited(screen))
    {
        gtk_widget_set_visual(window, visual);
    }
}

int main(int argc,
         char **argv)
{
    GtkWidget *window;
    GtkWidget *tray;
    int x, y;
    GdkDisplay *display;
    GdkRectangle monitor;
    GError *error = NULL;

    if (!gtk_init_with_args(&argc, &argv, NULL, NULL, NULL, &error))
    {
        fprintf(stderr, "%s", error->message);
        g_error_free(error);
        exit(1);
    }

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "Test Calendar");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /*
    gtk_widget_set_size_request (window, CALENDA_WIDTH, CALENDA_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
    gtk_window_set_deletable (GTK_WINDOW (window), FALSE);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_stick (GTK_WINDOW (window));
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    */
    gtk_widget_set_name(GTK_WIDGET(window), "trayWindow");

    //tran_setup (window);

    tray = kiran_tray_new();

    gtk_container_add(GTK_CONTAINER(window), tray);
    gtk_widget_show(tray);

    display = gdk_display_get_default();
    gdk_monitor_get_geometry(gdk_display_get_monitor(display, 0), &monitor);
    x = monitor.width - CALENDA_WIDTH;
    y = monitor.height - CALENDA_HEIGHT;
    //gtk_window_move (GTK_WINDOW (window), x, y);

    gtk_widget_show(window);

    gtk_main();

    return EXIT_SUCCESS;
}
