#pragma once

#include <wx/wx.h>

#include "drawingview.h"
#include "toolsettings.h"

class MyFrame;

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();

    static void SetupCanvasForView(DrawingView *view);
    static ToolSettings &GetToolSettings();

private:
    std::unique_ptr<wxDocManager> docManager;
    MyFrame *frame;

    ToolSettings toolSettings;
};
