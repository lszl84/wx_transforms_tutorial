#include <wx/graphics.h>

#include "drawingview.h"
#include "myapp.h"

#include "canvas/drawingcanvas.h"
#include "canvas/drawingvisitor.h"

#include "shapes/shapefactory.h"

#include "utils/visitor.h"

wxIMPLEMENT_DYNAMIC_CLASS(DrawingView, wxView);

bool DrawingView::OnCreate(wxDocument *doc, long flags)
{
    if (!wxView::OnCreate(doc, flags))
    {
        return false;
    }

    MyApp::SetupCanvasForView(this);

    return true;
}

bool DrawingView::OnClose(bool deleteWindow)
{
    if (deleteWindow)
    {
        MyApp::SetupCanvasForView(nullptr);
    }
    return wxView::OnClose(deleteWindow);
}

void DrawingView::OnChangeFilename()
{
    wxString appName = wxTheApp->GetAppDisplayName();
    wxString title;

    wxString docName = GetDocument()->GetUserReadableName();
    title = docName + (GetDocument()->IsModified() ? " - Edited" : "") + wxString(_(" - ")) + appName;

    GetFrame()->SetLabel(title);
}

void DrawingView::OnDraw(wxDC *dc)
{
    dc->SetBackground(*wxWHITE_BRUSH);
    dc->Clear();

    std::unique_ptr<wxGraphicsContext> gc{wxGraphicsContext::CreateFromUnknownDC(*dc)};

    if (gc)
    {
        DrawingVisitor drawingVisitor{*gc};

        for (const auto &s : GetDocument()->shapes)
        {
            std::visit(drawingVisitor, s);
        }
    }
}

void DrawingView::OnMouseDown(wxPoint pt)
{
    lastDragStart = pt;

    GetDocument()->shapes.push_back(ShapeFactory::Create(MyApp::GetToolSettings(), pt));
    GetDocument()->Modify(true);
}

void DrawingView::OnMouseDrag(wxPoint pt)
{
    auto &shapeInCreation = GetDocument()->shapes.back();

    std::visit(visitor{[&](Path &path)
                       {
                           path.points.push_back(pt);
                       },
                       [&](Rect &rect)
                       {
                           auto left = std::min(lastDragStart.x, pt.x);
                           auto top = std::min(lastDragStart.y, pt.y);
                           auto right = std::max(lastDragStart.x, pt.x);
                           auto bottom = std::max(lastDragStart.y, pt.y);

                           rect.rect.SetLeft(left);
                           rect.rect.SetTop(top);
                           rect.rect.SetRight(right);
                           rect.rect.SetBottom(bottom);
                       },
                       [&](Circle &circle)
                       {
                           circle.radius = std::sqrt(std::pow(pt.x - circle.center.m_x, 2) + std::pow(pt.y - circle.center.m_y, 2));
                       }},
               shapeInCreation);

    GetDocument()->Modify(true);
}

void DrawingView::OnMouseDragEnd()
{
    // Nothing to do here
}

void DrawingView::OnClear()
{
    GetDocument()->shapes.clear();
    GetDocument()->Modify(true);
}

DrawingDocument *DrawingView::GetDocument() const
{
    return wxStaticCast(wxView::GetDocument(), DrawingDocument);
}