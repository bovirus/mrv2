// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <sstream>

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "mrvCore/mrvI8N.h"

#ifdef TLRENDER_USD
#    include "mrvOptions/mrvUSD.h"
#endif

void mrv2_usd(pybind11::module& m)
{

    py::module usd = m.def_submodule("usd");
    usd.doc() = _(R"PYTHON(
USD module.

Contains all classes and enums related to USD (Universal Scene Description). 
)PYTHON");

#ifdef TLRENDER_USD

    using namespace mrv;

    py::class_<usd::RenderOptions>(usd, "RenderOptions")
        .def(py::init<>())
        .def(
            py::init<
                std::string, int, float, tl::usd::DrawMode, bool, bool, bool,
                bool, size_t, size_t>(),
            py::arg("rendererName") = 0, py::arg("renderWidth") = 1920,
            py::arg("complexity") = 1.F,
            py::arg("drawMode") = tl::usd::DrawMode::ShadedSmooth,
            py::arg("enableLighting") = true,
            py::arg("enableSceneLights") = false,
            py::arg("enableSceneMaterials") = false, py::arg("sRGB") = true,
            py::arg("stageCache") = 10, py::arg("diskCache") = 0)
        .def_readwrite(
            "rendererName", &usd::RenderOptions::rendererName,
            _("Renderer Name"))
        .def_readwrite(
            "renderWidth", &usd::RenderOptions::renderWidth, _("Render Width"))
        .def_readwrite(
            "complexity", &usd::RenderOptions::complexity,
            _("Models' complexity."))
        .def_readwrite(
            "drawMode", &usd::RenderOptions::drawMode,
            _("Draw mode  :class:`mrv2.usd.DrawMode`."))
        .def_readwrite(
            "enableLighting", &usd::RenderOptions::enableLighting,
            _("Enable Lighting"))
        .def_readwrite(
            "enableSceneLights", &usd::RenderOptions::enableLighting,
            _("Enable Scene Lights"))
        .def_readwrite(
            "enableSceneMaterials", &usd::RenderOptions::enableLighting,
            _("Enable Scene Materials"))
        .def_readwrite("sRGB", &usd::RenderOptions::sRGB, _("Enable sRGB"))
        .def_readwrite(
            "stageCache", &usd::RenderOptions::stageCache,
            _("Stage Cache Count"))
        .def_readwrite(
            "diskCache", &usd::RenderOptions::diskCache,
            _("Disk Cache Byte Count"))
        .def(
            "__repr__",
            [](const usd::RenderOptions& o)
            {
                std::stringstream s;
                s << "<mrv2.usd.RenderOptions renderWidth=" << o.renderWidth
                  << " complexity=" << o.complexity
                  << " drawMode=" << o.drawMode
                  << " enableLighting=" << (o.enableLighting ? "True" : "False")
                  << " sRGB=" << (o.sRGB ? "True" : "False")
                  << " stageCache=" << o.stageCache
                  << " diskCache=" << o.diskCache << ">";
                return s.str();
            })
        .doc() = _("USD Render Options.");

    usd.def(
        "renderOptions", &mrv::usd::renderOptions,
        _("Get USD Render Options."));

    usd.def(
        "setRenderOptions", &mrv::usd::setRenderOptions,
        _("Set USD Render Options."), py::arg("renderOptions"));
#endif
}
