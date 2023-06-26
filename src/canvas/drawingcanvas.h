#pragma once

#include <wx/wx.h>
#include <vector>

#include "../drawingview.h"

class DrawingCanvas : public wxWindow
{
public:
    DrawingCanvas(wxWindow *parent, DrawingView *view, wxWindowID id, const wxPoint &pos, const wxSize &size);
    virtual ~DrawingCanvas() noexcept {}

    void ShowExportDialog();
    void SetView(DrawingView *view);

private:
    void OnPaint(wxPaintEvent &);
    void DrawOnContext(wxGraphicsContext *gc);

    void OnMouseDown(wxMouseEvent &);
    void OnMouseMove(wxMouseEvent &);
    void OnMouseUp(wxMouseEvent &);
    void OnMouseLeave(wxMouseEvent &);

    DrawingView *view;

    bool isDragging{false};

    wxMenu contextMenu;
    void BuildContextMenu();
    void OnContextMenuEvent(wxContextMenuEvent &);
};