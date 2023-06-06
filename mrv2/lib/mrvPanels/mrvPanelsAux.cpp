#include <tlCore/StringFormat.h>

#include "mrvPanelsAux.h"

#include "mrViewer.h"

namespace mrv
{

    std::string getLayerName(int layerId, ViewerUI* ui)
    {
        std::string layer;
        if (layerId >= 0 && layerId < ui->uiColorChannel->children())
        {
            layer = "\n";
            layer += ui->uiColorChannel->child(layerId)->label();
        }
        else if (layerId >= ui->uiColorChannel->children())
        {
            layer = tl::string::Format("\n{0}").arg(layerId);
        }
        return layer;
    }

} // namespace mrv