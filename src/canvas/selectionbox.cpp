#include <array>

#include "selectionbox.h"
#include "objectspace.h"

#include <iostream>

enum class SelectionBox::DraggableElement
{
    TopLeftHandle,
    TopRightHandle,
    BottomRightHandle,
    BottomLeftHandle,
    RotationHandle,
    FullBox
};

void SelectionBox::Draw(wxGraphicsContext &gc) const
{
    gc.PushState();

    gc.SetTransform(gc.CreateMatrix()); // empty transform as the handle positions are already in screen coordinates

    gc.SetPen(wxPen(wxColour(128, 128, 128), 1));
    gc.SetBrush(*wxTRANSPARENT_BRUSH);

    auto rectVertices = std::array{
        GetTopLeftHandleCenter(),
        GetTopRightHandleCenter(),
        GetBottomRightHandleCenter(),
        GetBottomLeftHandleCenter(),
        GetTopLeftHandleCenter()};

    auto rotationHandleStart = GetRotationHandleStart();
    auto rotationHandleCenter = GetRotationHandleCenter();

    gc.StrokeLines(rectVertices.size(), rectVertices.data());
    gc.StrokeLine(rotationHandleStart.m_x, rotationHandleStart.m_y, rotationHandleCenter.m_x, rotationHandleCenter.m_y);

    auto handleCenters = std::array{
        GetTopLeftHandleCenter(),
        GetTopRightHandleCenter(),
        GetBottomRightHandleCenter(),
        GetBottomLeftHandleCenter(),
        GetRotationHandleCenter()};

    for (auto center : handleCenters)
    {
        DrawHandle(gc, center);
    }

    gc.PopState();
}

void SelectionBox::DrawHandle(wxGraphicsContext &gc, wxPoint2DDouble center) const
{
    gc.PushState();

    gc.Translate(center.m_x, center.m_y);
    gc.Rotate(object.get().transformation.rotationAngle);

    gc.SetPen(*wxRED_PEN);
    gc.SetBrush(*wxRED_BRUSH);
    gc.DrawRectangle(-handleWidth / 2, -handleWidth / 2, handleWidth, handleWidth);

    gc.PopState();
}

void SelectionBox::StartDragIfClicked(wxPoint2DDouble pt)
{
    if (HandleHitTest(pt, GetRotationHandleCenter()))
    {
        draggedElement = DraggableElement::RotationHandle;
    }
    else if (HandleHitTest(pt, GetTopLeftHandleCenter()))
    {
        draggedElement = DraggableElement::TopLeftHandle;
    }
    else if (HandleHitTest(pt, GetTopRightHandleCenter()))
    {
        draggedElement = DraggableElement::TopRightHandle;
    }
    else if (HandleHitTest(pt, GetBottomRightHandleCenter()))
    {
        draggedElement = DraggableElement::BottomRightHandle;
    }
    else if (HandleHitTest(pt, GetBottomLeftHandleCenter()))
    {
        draggedElement = DraggableElement::BottomLeftHandle;
    }
    else if (FullBoxHitTest(pt))
    {
        draggedElement = DraggableElement::FullBox;
    }
    else
    {
        draggedElement = {};
    }

    lastDragPoint = pt;
}

bool SelectionBox::IsDragging() const
{
    return draggedElement.has_value();
}

void SelectionBox::Drag(wxPoint2DDouble pt)
{
    if (!IsDragging())
    {
        throw std::runtime_error("Drag called without StartDragIfClicked");
    }

    switch (draggedElement.value())
    {
    case DraggableElement::TopLeftHandle:
        ScaleUsingHandleMovement(lastDragPoint, pt, GetTopLeftHandleCenter());
        break;
    case DraggableElement::TopRightHandle:
        ScaleUsingHandleMovement(lastDragPoint, pt, GetTopRightHandleCenter());
        break;
    case DraggableElement::BottomRightHandle:
        ScaleUsingHandleMovement(lastDragPoint, pt, GetBottomRightHandleCenter());
        break;
    case DraggableElement::BottomLeftHandle:
        ScaleUsingHandleMovement(lastDragPoint, pt, GetBottomLeftHandleCenter());
        break;
    case DraggableElement::RotationHandle:
        RotateUsingMovement(lastDragPoint, pt);
        break;
    case DraggableElement::FullBox:
        TranslateUsingMovement(lastDragPoint, pt);
        break;
    }

    lastDragPoint = pt;
}

void SelectionBox::ScaleUsingHandleMovement(wxPoint2DDouble dragStart, wxPoint2DDouble dragEnd, wxPoint2DDouble handleCenter)
{
    const auto directionFromCenter = ObjectSpace::ToObjectCoordinates(object.get(), handleCenter) - object.get().boundingBox.GetCentre();
    const auto dragInObjectSpace = ObjectSpace::ToObjectDistance(object.get(), dragEnd - dragStart);

    const auto [halfBoxWidth, halfBoxHeight] = object.get().boundingBox.GetSize() / 2;
    const auto halfWidthAdjustment = directionFromCenter.m_x > 0 ? dragInObjectSpace.m_x : -dragInObjectSpace.m_x;
    const auto halfHeightAdjustment = directionFromCenter.m_y > 0 ? dragInObjectSpace.m_y : -dragInObjectSpace.m_y;

    object.get().transformation.scaleX *= (halfBoxWidth + halfWidthAdjustment) / halfBoxWidth;
    object.get().transformation.scaleY *= (halfBoxHeight + halfHeightAdjustment) / halfBoxHeight;
}

void SelectionBox::RotateUsingMovement(wxPoint2DDouble dragStart, wxPoint2DDouble dragEnd)
{
    const auto objectCenterOnScreen = ObjectSpace::ToScreenCoordinates(object.get(), object.get().boundingBox.GetCentre());

    const auto v1 = dragStart - objectCenterOnScreen;
    const auto v2 = dragEnd - objectCenterOnScreen;

    const double dot = v1.m_x * v2.m_x + v1.m_y * v2.m_y;
    const double cross = v1.m_x * v2.m_y - v1.m_y * v2.m_x;
    const double angle = std::atan2(cross, dot);

    object.get().transformation.rotationAngle += angle;
}

void SelectionBox::TranslateUsingMovement(wxPoint2DDouble dragStart, wxPoint2DDouble dragEnd)
{
    const auto dragVector = dragEnd - dragStart;
    object.get().transformation.translationX += dragVector.m_x;
    object.get().transformation.translationY += dragVector.m_y;
}

void SelectionBox::FinishDrag()
{
    draggedElement = {};
}

bool SelectionBox::HandleHitTest(wxPoint2DDouble pt, wxPoint2DDouble handleCenter) const
{
    wxAffineMatrix2D screenToHandleMatrix;

    screenToHandleMatrix.Translate(handleCenter.m_x, handleCenter.m_y);
    screenToHandleMatrix.Rotate(object.get().transformation.rotationAngle);
    screenToHandleMatrix.Invert();

    return wxRect2DDouble(-handleWidth / 2, -handleWidth / 2, handleWidth, handleWidth)
        .Contains(screenToHandleMatrix.TransformPoint(pt));
}

bool SelectionBox::FullBoxHitTest(wxPoint2DDouble pt) const
{
    return object.get().boundingBox.Contains(
        ObjectSpace::ToObjectCoordinates(object.get(), pt));
}

wxPoint2DDouble SelectionBox::GetRotationHandleStart() const
{
    const auto box = object.get().boundingBox;
    return ObjectSpace::ToScreenCoordinates(object.get(), box.GetLeftBottom() + wxPoint2DDouble{box.m_width / 2.0, 0.0});
}

wxPoint2DDouble SelectionBox::GetRotationHandleCenter() const
{
    const auto box = object.get().boundingBox;
    const auto handleDistanceY = handleWidth * 2.0 / std::fabs(object.get().transformation.scaleY);
    const auto boxBottomCenter = box.GetLeftBottom() + wxPoint2DDouble{box.m_width / 2.0, 0.0};

    return ObjectSpace::ToScreenCoordinates(object.get(), {boxBottomCenter.m_x, boxBottomCenter.m_y + handleDistanceY});
}

wxPoint2DDouble SelectionBox::GetTopLeftHandleCenter() const
{
    return ObjectSpace::ToScreenCoordinates(object.get(), object.get().boundingBox.GetLeftTop());
}

wxPoint2DDouble SelectionBox::GetTopRightHandleCenter() const
{
    return ObjectSpace::ToScreenCoordinates(object.get(), object.get().boundingBox.GetRightTop());
}

wxPoint2DDouble SelectionBox::GetBottomLeftHandleCenter() const
{
    return ObjectSpace::ToScreenCoordinates(object.get(), object.get().boundingBox.GetLeftBottom());
}

wxPoint2DDouble SelectionBox::GetBottomRightHandleCenter() const
{
    return ObjectSpace::ToScreenCoordinates(object.get(), object.get().boundingBox.GetRightBottom());
}