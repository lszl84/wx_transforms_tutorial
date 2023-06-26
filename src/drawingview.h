#pragma once

#include <optional>

#include <wx/docview.h>

#include "drawingdocument.h"
#include "canvas/canvasobject.h"
#include "canvas/shapecreator.h"
#include "canvas/selectionbox.h"

class DrawingView : public wxView
{
public:
    bool OnCreate(wxDocument *doc, long flags) override;

    void OnDraw(wxDC *dc) override;

    void OnMouseDown(wxPoint);
    void OnMouseDrag(wxPoint);
    void OnMouseDragEnd();

    void OnClear();
    bool OnClose(bool deleteWindow = true) override;

    // Setting the Frame title
    void OnChangeFilename() override;

    DrawingDocument *GetDocument() const;

    ShapeCreator shapeCreator;
    std::optional<SelectionBox> selection;

    wxDECLARE_DYNAMIC_CLASS(DrawingView);
};