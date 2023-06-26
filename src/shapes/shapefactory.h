#pragma once

#include <stdexcept>

#include "shape.h"
#include "path.h"
#include "rect.h"
#include "circle.h"

#include "../toolsettings.h"

struct ShapeFactory
{
    static Shape Create(ToolSettings &settings, wxPoint origin)
    {
        wxPoint2DDouble origin2D = origin;

        switch (settings.currentTool)
        {
        case ToolType::Pen:
            return Path{{origin2D}, settings.currentColor, settings.currentWidth};
        case ToolType::Rect:
            return Rect{{origin2D.m_x, origin2D.m_y, 0, 0}, settings.currentColor};
        case ToolType::Circle:
            return Circle{0, origin2D, settings.currentColor};
        default:
            throw std::runtime_error("CreateObject: Unknown tool type");
        }
    }
};