#pragma once

#include <wx/graphics.h>

#include "../shapes/circle.h"
#include "../shapes/rect.h"
#include "../shapes/path.h"

struct DrawingVisitor
{
    wxGraphicsContext &gc;

    void operator()(const Circle &obj)
    {
        gc.SetPen(wxPen(obj.color));
        gc.SetBrush(wxBrush(obj.color));
        gc.DrawEllipse(obj.center.m_x - obj.radius, obj.center.m_y - obj.radius,
                       obj.radius * 2, obj.radius * 2);
    }

    void operator()(const Rect &obj)
    {
        gc.SetPen(wxPen(obj.color));
        gc.SetBrush(wxBrush(obj.color));
        gc.DrawRectangle(obj.rect.m_x, obj.rect.m_y, obj.rect.m_width, obj.rect.m_height);
    }

    void operator()(const Path &obj)
    {
        if (obj.points.size() > 1)
        {
            gc.SetPen(wxPen(obj.color, obj.width));
            gc.StrokeLines(obj.points.size(), obj.points.data());
        }
    }
};