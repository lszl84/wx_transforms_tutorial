#pragma once

#include <optional>

#include <wx/wx.h>

#include "canvasobject.h"
#include "../toolsettings.h"
#include "../shapes/shapefactory.h"
#include "../utils/visitor.h"
#include "drawingvisitor.h"

struct ShapeCreator
{
    void Start(ToolSettings toolSettings, wxPoint pt)
    {
        shape.emplace(ShapeFactory::Create(toolSettings, pt));
        lastDragStart = pt;
    }

    void Update(wxPoint pt)
    {
        if (!shape)
        {
            throw std::runtime_error("ShapeCreator::Update() called without a call to ShapeCreator::Start()");
        }

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
                   shape.value());
    }

    CanvasObject FinishAndGenerateObject()
    {
        if (!shape)
        {
            throw std::runtime_error("ShapeCreator::FinishAndGenerate() called without a call to ShapeCreator::Start()");
        }

        return CanvasObject{std::exchange(shape, {}).value()};
    }

    void Cancel()
    {
        shape = {};
    }

    bool IsCreating() const
    {
        return shape.has_value();
    }

    void Draw(wxGraphicsContext &gc)
    {
        if (shape)
        {
            std::visit(DrawingVisitor{gc}, shape.value());
        }
    }

private:
    std::optional<Shape> shape;
    wxPoint lastDragStart;
};