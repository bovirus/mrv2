#pragma once

#include <vector>
#include <string>

class Fl_Widget;
class Fl_Group;
class Fl_Menu_Item;
class Fl_Menu_;
class ViewerUI;

namespace mrv
{
    class TimelineViewport;
    class MainWindow;

    //! File menu callbacks
    void open_files_cb( const std::vector< std::string >& files,
                        ViewerUI* ui  );

    void open_cb( Fl_Widget* w, ViewerUI* ui );
    void open_directory_cb( Fl_Widget* w, ViewerUI* ui );
    void exit_cb( Fl_Widget* w, ViewerUI* ui );

    //! Display callbacks
    void display_options_cb( Fl_Menu_* m, TimelineViewport* view );

    void mirror_x_cb( Fl_Menu_* m, TimelineViewport* view );
    void mirror_y_cb( Fl_Menu_* m, TimelineViewport* view );

    //! Channel callbacks
    void toggle_red_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_green_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_blue_channel_cb( Fl_Menu_* m, TimelineViewport* view );
    void toggle_alpha_channel_cb( Fl_Menu_* m, TimelineViewport* view );

    //! Compare callbacks
    void change_media_cb( Fl_Menu_* m, MainWindow* w );

    void A_media_cb( Fl_Menu_* m, MainWindow* w );
    void B_media_cb( Fl_Menu_* m, MainWindow* w );

    void compare_wipe_cb( Fl_Menu_* m, MainWindow* w );
    void compare_overlay_cb( Fl_Menu_* m, MainWindow* w );
    void compare_difference_cb( Fl_Menu_* m, MainWindow* w );
    void compare_horizontal_cb( Fl_Menu_* m, MainWindow* w );
    void compare_vertical_cb( Fl_Menu_* m, MainWindow* w );
    void compare_tile_cb( Fl_Menu_* m, MainWindow* w );

    //! Window callbacks
    void window_cb( Fl_Menu_* w, ViewerUI* ui );

    //! HUD togle callback
    void hud_cb( Fl_Menu_* w, ViewerUI* ui );

    //! Auxiliary functions to remember what bars and what windows were
    //1 open in case of a fullscreen or presentation switch.
    void save_ui_state( ViewerUI* ui );
    void toggle_action_tool_bar( Fl_Menu_*, ViewerUI* ui );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar,
                        const int sizeX, const int sizeY );
    void toggle_ui_bar( ViewerUI* ui, Fl_Group* const bar, const int size );
    void hide_ui_state( ViewerUI* ui );
    void restore_ui_state( ViewerUI* ui );

    // Playback callbacks
    void play_forwards_cb( Fl_Menu_*, ViewerUI* ui );
    void play_backwards_cb( Fl_Menu_*, ViewerUI* ui );
    void stop_cb( Fl_Menu_*, ViewerUI* ui );
    void toggle_playback_cb( Fl_Menu_*, ViewerUI* ui );

    void playback_loop_cb( Fl_Menu_*, ViewerUI* ui );
    void playback_once_cb( Fl_Menu_*, ViewerUI* ui );
    void playback_ping_pong_cb( Fl_Menu_*, ViewerUI* ui );
}
