#include <wx/graphics.h>

#include "drawingview.h"
#include "myapp.h"
#include "canvas/drawingcanvas.h"

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
        for (const auto &obj : GetDocument()->objects)
        {
            obj.Draw(*gc);
        }

        if (selection)
        {
            selection->Draw(*gc);
        }

        shapeCreator.Draw(*gc);
    }
}

void DrawingView::OnMouseDown(wxPoint pt)
{
    if (MyApp::GetToolSettings().currentTool == ToolType::Transform)
    {
        // prioritize current selection handles hit test
        if (selection.has_value())
        {
            selection->StartDragIfClicked(pt);
        }

        bool clickedOnCurrentSelection = selection.has_value() && selection->IsDragging();

        if (!clickedOnCurrentSelection)
        {
            // set selection to clicked object or clear selection if clicked on empty space
            auto iterator = std::find_if(GetDocument()->objects.rbegin(), GetDocument()->objects.rend(), [&](auto &object)
                                         { return object.boundingBox.Contains(ObjectSpace::ToObjectCoordinates(object, pt)); });

            selection = iterator != GetDocument()->objects.rend() ? std::make_optional(SelectionBox{*iterator, MyApp::GetToolSettings().selectionHandleWidth}) : std::nullopt;

            // immediately start dragging if clicked on object
            if (selection.has_value())
            {
                selection->StartDragIfClicked(pt);
            }
        }
    }
    else
    {
        selection = {};
        shapeCreator.Start(MyApp::GetToolSettings(), pt);
    }
}

void DrawingView::OnMouseDrag(wxPoint pt)
{
    if (MyApp::GetToolSettings().currentTool == ToolType::Transform)
    {
        if (selection.has_value() && selection->IsDragging())
        {
            selection->Drag(pt);
            GetDocument()->Modify(true);
        }
    }
    else
    {
        shapeCreator.Update(pt);
    }
}

void DrawingView::OnMouseDragEnd()
{
    if (MyApp::GetToolSettings().currentTool == ToolType::Transform)
    {
        if (selection.has_value())
        {
            selection->FinishDrag();
        }
    }
    else
    {
        selection = {};
        GetDocument()->objects.push_back(shapeCreator.FinishAndGenerateObject());
        GetDocument()->Modify(true);
    }
}

void DrawingView::OnClear()
{
    selection = {};
    GetDocument()->objects.clear();
    GetDocument()->Modify(true);
}

DrawingDocument *DrawingView::GetDocument() const
{
    return wxStaticCast(wxView::GetDocument(), DrawingDocument);
}