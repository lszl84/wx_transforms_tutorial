#pragma once

#include "selectablepane.h"
#include "../toolsettings.h"

class ToolSelectionPane : public SelectablePane
{
public:
    ToolSelectionPane(wxWindow *parent, wxWindowID id, ToolType type, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

    ToolType toolType;

private:
    virtual void DrawContent(wxGraphicsContext *gc, const wxRect &rect, int roundness) const override;
};

ToolSelectionPane::ToolSelectionPane(wxWindow *parent, wxWindowID id, ToolType type, const wxPoint &pos, const wxSize &size)
    : SelectablePane(parent, id, pos, size), toolType(type)
{
}

void ToolSelectionPane::DrawContent(wxGraphicsContext *gc, const wxRect &rect, int roundness) const
{
    gc->SetPen(*wxWHITE_PEN);
    gc->SetBrush(*wxWHITE_BRUSH);

    gc->DrawRoundedRectangle(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), roundness);

    double itemWidth = rect.GetWidth() / 2.0;

    wxAffineMatrix2D transform{};
    transform.Translate(rect.GetX(), rect.GetY());
    transform.Translate(rect.GetWidth() / 2, rect.GetHeight() / 2);

    gc->PushState();
    gc->SetTransform(gc->CreateMatrix(transform));

    const auto color = wxColor(80, 80, 80);

    gc->SetPen(wxPen(color, FromDIP(3)));
    gc->SetBrush(wxBrush(color));

    switch (toolType)
    {
    case ToolType::Pen:
        // draw sine wave
        {
            std::vector<wxPoint2DDouble> points{};
            constexpr int Resolution = 100;
            for (int i = 0; i < Resolution; i++)
            {
                double x = (i / static_cast<double>(Resolution) - 0.5) * itemWidth;
                double y = sin(x / itemWidth * M_PI * 2.0) * itemWidth / 2.0;
                points.push_back(wxPoint2DDouble(x, y));
            }
            gc->Rotate(M_PI / 4.0);
            gc->StrokeLines(points.size(), points.data());
            break;
        }
    case ToolType::Rect:
        gc->DrawRectangle(-itemWidth / 2, -itemWidth / 2, itemWidth, itemWidth);
        break;
    case ToolType::Circle:
        gc->DrawEllipse(-itemWidth / 2, -itemWidth / 2, itemWidth, itemWidth);
        break;
    }

    gc->PopState();
}