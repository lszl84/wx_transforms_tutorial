#pragma once

#include <wx/wx.h>

enum class ToolType
{
    Pen,
    Rect,
    Circle,
    Transform
};

struct ToolSettings
{
    int currentWidth{1};
    wxColour currentColor{*wxBLACK};

    ToolType currentTool{ToolType::Pen};

    double selectionHandleWidth; // setup this with FromDIP
};