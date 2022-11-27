#pragma once

#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{   
    class LogsTool : public ToolWidget
    {
    public:
        LogsTool( ViewerUI* ui );
        ~LogsTool();

        void add_controls() override;
        void dock() override;
        void undock() override;

        void info( const std::string& msg ) const;
        void warning( const std::string& msg ) const;
        void error( const std::string& msg ) const;
        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
