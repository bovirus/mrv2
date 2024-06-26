// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>

#include "mrvDropWindow.h"

#ifdef __APPLE__
#    include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class Fl_Menu_;

class ViewerUI;

namespace mrv
{

    class App;

    class MainWindow : public DropWindow
    {
    public:
        MainWindow(int X, int Y, int W, int H, const char* L = 0);
        MainWindow(int W, int H, const char* L = 0);
        ~MainWindow();

        //! Initialize the window
        void init();

        //! Fill menu based on context information
        void fill_menu(Fl_Menu_* menu);

        //! Update title bar
        void update_title_bar();

        //! Make window appear always on top of others
        void always_on_top(int above);

        //! Returns whether the window is on top of all others.
        bool is_on_top() const { return on_top; }

        //! Change window's icon to mrViewer's icon
        void set_icon();

        //! Iconize all windows
        void iconize_all();

        //! Handle override.
        int handle(int e) FL_OVERRIDE;

        //! Resize override to handle tile.
        void resize(int X, int Y, int W, int H) FL_OVERRIDE;

        //! Return whether we are resizing under wayland.
        bool is_wayland_resize() const { return wayland_resize; }
        
    protected:
#ifdef __APPLE__
        IOPMAssertionID assertionID;
        IOReturn success;
#endif
        bool wayland_resize = false;
        bool on_top = false;
    };

} // namespace mrv
