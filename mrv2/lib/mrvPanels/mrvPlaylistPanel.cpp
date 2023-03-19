// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>

#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace mrv
{

    typedef std::map< ClipButton*, int64_t > WidgetIds;

    struct PlaylistPanel::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator* thumbnailCreator;

        std::vector< ClipButton* > clipButtons;

        size_t index = 0;
        std::vector< Playlist > playlists;

        std::vector< std::shared_ptr<FilesModelItem> > clips;

        WidgetIds ids;
        std::vector< Fl_Button* > buttons;


        Fl_Choice* playlistId;
        
        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
    };

    struct ThumbnailData
    {
        ClipButton* widget;
    };

    void playlistThumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        void* opaque)
    {
        ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
        ClipButton* w = data->widget;
        if (playlistPanel)
            playlistPanel->playlistThumbnail(id, thumbnails, w);
        delete data;
    }

    void PlaylistPanel::playlistThumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        ClipButton* w)
    {
        WidgetIds::const_iterator it = _r->ids.find(w);
        if (it == _r->ids.end())
        {
            std::cerr << "widget not found for id " << id << std::endl;
            return;
        }
        
        if (it->second == id)
        {
            for (const auto& i : thumbnails)
            {
                Fl_Image* img = w->image();
                w->image(i.second);
                delete img;
                w->redraw();
            }
        }
        else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    PlaylistPanel::PlaylistPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {
        _r->context = ui->app->getContext();

        add_group("Playlist");

        // Fl_SVG_Image* svg = load_svg( "Playlist.svg" );
        // g->image( svg );

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete playlistPanel;
                playlistPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);

        _r->filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                ui->app->filesModel()->observeFiles(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { refresh(); });
    }

    PlaylistPanel::~PlaylistPanel()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void PlaylistPanel::clear_controls()
    {
        _r->clipButtons.clear();
    }

    void PlaylistPanel::cancel_thumbnails()
    {
        for (const auto& it : _r->ids)
        {
            _r->thumbnailCreator->cancelRequests(it.second);
        }

        _r->ids.clear();
    }

    void PlaylistPanel::add_controls()
    {
        TLRENDER_P();

        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();

        g->clear();
        g->begin();

        // Make sure all clips exist in file liest
        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles().get()->get();

        // If playlists is empty, create a new dummy playlist
        if ( _r->playlists.empty() )
        {
            Playlist playlist;
            playlist.name = "Playlist";
            _r->playlists.push_back( playlist );
        }
        //
        // Make sure all playlists do not have an item that no longer exists
        //
        for ( auto& playlist : _r->playlists )
        {
            std::vector< std::shared_ptr<FilesModelItem> > newclips;
            for (size_t i = 0; i < playlist.clips.size(); ++i)
            {
                auto clip = playlist.clips[i];
                if (std::find(files.begin(), files.end(), clip) != files.end())
                    newclips.push_back(clip);
            }
            playlist.clips = newclips;
        }

        int index = _r->index;
        int Y = g->y() + 20;
        int W = g->w();

        Pack*   pack = new Pack(g->x(), Y, W, 30);
        pack->type( Pack::HORIZONTAL );
        pack->begin();
        auto cW = new Widget< Fl_Choice >( g->x() + W - 60, Y, W - 60, 30,
                                           _("Name") );
        Fl_Choice* c = _r->playlistId = cW;
        for ( auto playlist : _r->playlists )
        {
            c->add( playlist.name.c_str() );
        }
        c->value( _r->index );
        cW->callback([=](auto b)
            {
                _r->index = b->value();
                refresh();
            });

        auto bW = new Widget< Button >( g->x() + W - 60, Y, 30, 30, "+" );
        Button* b = bW;
        bW->callback([=](auto b)
            {
                Playlist playlist;
                char name[256];
                snprintf( name, 256, "Playlist_%d", c->size() ); 
                playlist.name = name;
                _r->playlists.push_back( playlist );
                _r->index = _r->playlists.size() - 1;
                refresh();
            });
        
        bW = new Widget< Button >( g->x() + W - 30, Y, 30, 30, "-" );
        b = bW;
        bW->callback([=](auto b)
            {
                int index = _r->index;
                _r->playlists.erase( _r->playlists.begin() + index );
                if ( index >= _r->playlists.size() )
                    _r->index = index-1;
                refresh();
            });

        
        pack->end();
        pack->layout();
        
        size_t numFiles = _r->playlists[index].clips.size();

        imaging::Size size(128, 64);

        for (size_t i = 0; i < numFiles; ++i)
        {
            const auto& media = _r->playlists[index].clips[i];
            const auto& path = media->path;

            const std::string& fullfile = path.get();

            auto cbW = new Widget<ClipButton>(
                g->x(), g->y() + 20 + i * 68, g->w(), 68);
            ClipButton* b = cbW;
            _r->clipButtons.push_back(b);
            cbW->callback([=](auto b) { b->value(!b->value()); });

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            std::string text = dir + "\n" + file;
            b->copy_label(text.c_str());

            if (auto context = _r->context.lock())
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget = b;

                const auto& timeRange = media->inOutRange;
                const auto& time = timeRange.start_time();

                _r->thumbnailCreator->initThread();
                try
                {
                    int64_t id = _r->thumbnailCreator->request(
                        fullfile, time, size, playlistThumbnail_cb,
                        (void*)data);
                    _r->ids[b] = id;
                }
                catch (const std::exception&)
                {
                }
            }
        }

        Y += 30 + numFiles * 64;

        Fl_Pack* bg = new Fl_Pack(g->x(), Y, g->w(), 30);

        bg->type(Fl_Pack::HORIZONTAL);
        bg->begin();

        bW = new Widget< Button >(g->x() + 150, Y, 50, 30, "Add");
        b = bW;
        b->tooltip(_("Add current file to Playlist"));
        bW->callback(
            [=](auto w)
            {
                const auto& model = p.ui->app->filesModel();
                const auto& files = model->observeFiles();
                if (files->getSize() == 0)
                    return;
                const auto Aindex = model->observeAIndex()->get();
                const auto& item = files->getItem(Aindex);
                auto clip = std::make_shared<FilesModelItem>();
                clip = item;
                const auto& player = p.ui->uiView->getTimelinePlayer();
                clip->inOutRange = player->inOutRange();
                int index = _r->playlistId->value();
                Playlist& playlist = _r->playlists[index];
                playlist.clips.push_back(clip);
                refresh();
            });

        bW = new Widget< Button >(g->x() + 150, Y, 70, 30, "Remove");
        b = bW;
        b->tooltip(_("Remove selected files from Playlist"));
        bW->callback(
            [=](auto w)
            {
                // Create a new list of new clips not taking into account
                // those selected (to remove)
                int index = _r->playlistId->value();
                Playlist& playlist = _r->playlists[index];
                std::vector< std::shared_ptr<FilesModelItem> > newclips;
                for (size_t i = 0; i < playlist.clips.size(); ++i)
                {
                    if (!_r->clipButtons[i]->value())
                        newclips.push_back(playlist.clips[i]);
                }
                playlist.clips = newclips;
                refresh();
            });

        bW = new Widget< Button >(g->x() + 150, Y, 70, 30, _("Create"));
        b = bW;
        b->tooltip(_("Create .otio Playlist"));
        bW->callback([=](auto w)
            {
                int index = _r->playlistId->value();
                Playlist& playlist = _r->playlists[index];
                if ( playlist.clips.size() < 2)
                    return;
                create_playlist(p.ui, playlist, true);
            });

        bW = new Widget< Button >(g->x() + 150, Y, 70, 30, _("Save"));
        b = bW;
        b->tooltip(_("Create and save an .otio Playlist with relative paths."));
        bW->callback(
            [=](auto w)
            {
                int index = _r->playlistId->value();
                Playlist& playlist = _r->playlists[index];
                if ( playlist.clips.size() < 2)
                    return;
                create_playlist(p.ui, playlist, false);
            });
        
        bg->end();

        g->end();
    }

    void PlaylistPanel::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }
    
    void PlaylistPanel::setPlaylists(const std::vector< Playlist >& value)
    {
        _r->playlists = value;
        refresh();
    }
    
    void PlaylistPanel::setPlaylist(const size_t idx, const Playlist& value)
    {
        if ( idx >= _r->playlists.size() )
            return;
        
        _r->playlists[idx] = value;
        refresh();
    }
        
} // namespace mrv
