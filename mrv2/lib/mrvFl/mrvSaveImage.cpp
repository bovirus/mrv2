// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGL/Init.h>
#include <tlGL/Util.h>
#include <tlTimelineGL/Render.h>

#include "mrvCore/mrvLocale.h"
#include "mrvCore/mrvMath.h"
#include "mrvCore/mrvUtil.h"

#include "mrvWidgets/mrvProgressReport.h"

#include "mrvGL/mrvGLErrors.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvFl/mrvSaveOptions.h"
#include "mrvFl/mrvIO.h"

#include "mrViewer.h"

#include <FL/platform.H>
#undef None

namespace
{
    const char* kModule = "save";
}

namespace mrv
{

    int save_single_frame(
        const std::string& file, const ViewerUI* ui, SaveOptions options)
    {
        std::string msg;

        int ret = 0;
        Viewport* view = ui->uiView;
        bool presentation = view->getPresentationMode();
        bool hud = view->getHudActive();

        auto player = view->getTimelinePlayer();
        if (!player)
            return -1; // should never happen

        file::Path path(file);

        if (file::isTemporaryEDL(path))
        {
            LOG_ERROR(_("Cannot save an NDI stream"));
            return -1;
        }

        // Stop the playback
        player->stop();

        // Time range.
        auto currentTime = player->currentTime();

        const std::string& extension = path.getExtension();

        bool saveEXR = string::compare(
            extension, ".exr", string::Compare::CaseInsensitive);
        bool saveHDR = string::compare(
            extension, ".hdr", string::Compare::CaseInsensitive);

        try
        {

            tl::io::Options ioOptions;

            char buf[256];

#ifdef TLRENDER_EXR
            ioOptions["OpenEXR/Compression"] = getLabel(options.exrCompression);
            ioOptions["OpenEXR/PixelType"] = getLabel(options.exrPixelType);
            snprintf(buf, 256, "%d", options.zipCompressionLevel);
            ioOptions["OpenEXR/ZipCompressionLevel"] = buf;
            {
                std::stringstream s;
                s << options.dwaCompressionLevel;
                ioOptions["OpenEXR/DWACompressionLevel"] = s.str();
            }
#endif

            otime::TimeRange oneFrameTimeRange(
                currentTime, otime::RationalTime(1, currentTime.rate()));

            auto context = ui->app->getContext();
            auto timeline = player->timeline();

            // Render information.
            const auto& info = player->ioInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video information");
            }

            gl::OffscreenBufferOptions offscreenBufferOptions;
            std::shared_ptr<timeline_gl::Render> render;
            image::Size renderSize;

            int layerId = ui->uiColorChannel->value();
            if (layerId < 0)
                layerId = 0;

            const SaveResolution resolution = options.resolution;
            {
                renderSize = info.video[layerId].size;
                auto rotation = ui->uiView->getRotation();
                if (options.annotations && rotationSign(rotation) != 0)
                {
                    size_t tmp = renderSize.w;
                    renderSize.w = renderSize.h;
                    renderSize.h = tmp;

                    msg = tl::string::Format(_("Rotated image info: {0}"))
                              .arg(renderSize);
                    LOG_INFO(msg);
                }
                if (resolution == SaveResolution::kHalfSize)
                {
                    renderSize.w /= 2;
                    renderSize.h /= 2;
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_INFO(msg);
                }
                else if (resolution == SaveResolution::kQuarterSize)
                {
                    renderSize.w /= 4;
                    renderSize.h /= 4;
                    msg = tl::string::Format(_("Scaled image info: {0}"))
                              .arg(renderSize);
                    LOG_INFO(msg);
                }
            }

            // Create the renderer.
            render = timeline_gl::Render::create(context);
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;

            // Create the writer.
            auto writerPlugin =
                context->getSystem<io::System>()->getPlugin(path);

            if (!writerPlugin)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open writer plugin.")
                        .arg(file));
            }

            int X = 0, Y = 0;

            io::Info ioInfo;
            image::Info outputInfo;

            outputInfo.size = renderSize;
            std::shared_ptr<image::Image> outputImage;

            outputInfo.pixelType = info.video[layerId].pixelType;

            {
                std::string msg = tl::string::Format(_("Image info: {0} {1}"))
                                      .arg(outputInfo.size)
                                      .arg(outputInfo.pixelType);
                LOG_INFO(msg);

                if (options.annotations)
                {
                    view->setActionMode(ActionMode::kScrub);
                    view->setPresentationMode(true);
                    view->redraw();
                    // flush is needed
                    Fl::flush();
                    view->flush();
                    Fl::check();
                    const auto& viewportSize = view->getViewportSize();
                    math::Size2i outputSize;
                    if (viewportSize.w >= renderSize.w &&
                        viewportSize.h >= renderSize.h)
                    {
                        view->setFrameView(false);
                        view->setViewZoom(1.0);
                        view->centerView();
                        view->redraw();
                        // flush is needed
                        Fl::flush();

                        outputInfo.size = renderSize;
                    }
                    else
                    {
                        LOG_WARNING(_("Image too big for options.annotations.  "
                                      "Will scale to the viewport size."));

                        view->frameView();

                        double viewportRatio =
                            viewportSize.w /
                            static_cast<double>(viewportSize.h);
                        double imageRatio =
                            renderSize.w / static_cast<double>(renderSize.h);

                        if (imageRatio < viewportRatio)
                        {
                            double factor = viewportSize.h /
                                            static_cast<double>(renderSize.h);
                            outputInfo.size.w =
                                std::round(renderSize.w * factor);
                            outputInfo.size.h = viewportSize.h;
                        }
                        else
                        {
                            double factor = viewportSize.w /
                                            static_cast<double>(renderSize.w);
                            outputInfo.size.h =
                                std::round(renderSize.h * factor);
                            outputInfo.size.w = viewportSize.w;
                        }
                    }

                    X = (viewportSize.w - outputInfo.size.w) / 2;
                    Y = (viewportSize.h - outputInfo.size.h) / 2;

                    msg = tl::string::Format(_("Viewport Size: {0} - "
                                               "X={1}, Y={2}"))
                              .arg(viewportSize)
                              .arg(X)
                              .arg(Y);
                    LOG_INFO(msg);
                }
            }

            outputInfo = writerPlugin->getWriteInfo(outputInfo);
            if (image::PixelType::None == outputInfo.pixelType)
            {
                outputInfo.pixelType = image::PixelType::RGB_U8;
                offscreenBufferOptions.colorType = image::PixelType::RGB_U8;
#ifdef TLRENDER_EXR
                if (saveEXR)
                {
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGB_F32;
                }
#endif
                msg = tl::string::Format(
                          _("Writer plugin did not get output info.  "
                            "Defaulting to {0}"))
                          .arg(offscreenBufferOptions.colorType);
                LOG_INFO(msg);
            }

#ifdef TLRENDER_EXR
            if (saveEXR)
            {
                outputInfo.pixelType = options.exrPixelType;
            }
#endif
            if (saveHDR)
            {
                outputInfo.pixelType = image::PixelType::RGB_F32;
                offscreenBufferOptions.colorType = image::PixelType::RGB_F32;
            }

            std::string msg = tl::string::Format(_("Output info: {0} {1}"))
                                  .arg(outputInfo.size)
                                  .arg(outputInfo.pixelType);
            LOG_INFO(msg);

            outputImage = image::Image::create(outputInfo);
            ioInfo.video.push_back(outputInfo);
            ioInfo.videoTime = oneFrameTimeRange;

            auto writer = writerPlugin->write(path, ioInfo, ioOptions);
            if (!writer)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            // Don't send any tcp updates
            tcp->lock();

            const GLenum format = gl::getReadPixelsFormat(outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot open").arg(file));
            }

            {
                std::string msg = tl::string::Format(_("OpenGL info: {0}"))
                                      .arg(offscreenBufferOptions.colorType);
                LOG_INFO(msg);
            }

            // Turn off hud so it does not get captured by glReadPixels.
            view->setHudActive(false);

            math::Size2i offscreenBufferSize(renderSize.w, renderSize.h);

            view->make_current();
            gl::initGLAD();

            auto buffer = gl::OffscreenBuffer::create(
                offscreenBufferSize, offscreenBufferOptions);

            if (options.annotations)
            {
                // Refresh the view (so we turn off the HUD, for example).
                view->redraw();
                view->flush();

                GLenum imageBuffer = GL_FRONT;

#ifdef FLTK_USE_WAYLAND
                // @note: Wayland does not work like Windows, macOS or
                //        X11.  The compositor does not immediately
                //        swap buffers when calling view->flush().
                if (fl_wl_display())
                {
                    imageBuffer = GL_BACK;
                }
#else
                view->make_current();
#endif
                glReadBuffer(imageBuffer);

                CHECK_GL;
                glReadPixels(
                    X, Y, outputInfo.size.w, outputInfo.size.h, format, type,
                    outputImage->getData());
                CHECK_GL;
            }
            else
            {
                // Get the videoData
                const auto& videoData =
                    timeline->getVideo(currentTime).future.get();

                view->make_current();
                gl::initGLAD();

                // Render the video.
                gl::OffscreenBufferBinding binding(buffer);
                CHECK_GL;
                {
                    locale::SetAndRestore saved;
                    render->begin(offscreenBufferSize);
                    render->setOCIOOptions(view->getOCIOOptions());
                    render->setLUTOptions(view->lutOptions());
                    CHECK_GL;
                    render->drawVideo(
                        {videoData},
                        {math::Box2i(0, 0, renderSize.w, renderSize.h)},
                        {timeline::ImageOptions()},
                        {timeline::DisplayOptions()},
                        timeline::CompareOptions(),
                        ui->uiView->getBackgroundOptions());
                    CHECK_GL;
                    render->end();
                }

                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                CHECK_GL;

                glPixelStorei(GL_PACK_ALIGNMENT, outputInfo.layout.alignment);
                CHECK_GL;
                glPixelStorei(
                    GL_PACK_SWAP_BYTES,
                    outputInfo.layout.endian != memory::getEndian());
                CHECK_GL;

                glReadPixels(
                    0, 0, outputInfo.size.w, outputInfo.size.h, format, type,
                    outputImage->getData());
                CHECK_GL;
            }

            writer->writeVideo(currentTime, outputImage);

            // Rename file name that got saved with a frame number to the actual
            // frame that the user set.
            int64_t currentFrame = currentTime.to_frames();
            char filename[4096];
            snprintf(
                filename, 4096, "%s%s%0*" PRId64 "%s",
                path.getDirectory().c_str(), path.getBaseName().c_str(),
                static_cast<int>(path.getPadding()), currentFrame,
                path.getExtension().c_str());

            if (filename != file && !options.noRename)
            {
                fs::rename(fs::path(filename), fs::path(file));
            }
            tcp->unlock();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
            ret = -1;
        }

        view->setFrameView(ui->uiPrefs->uiPrefsAutoFitImage->value());
        view->setHudActive(hud);
        view->setPresentationMode(presentation);
        return ret;
    }

} // namespace mrv
