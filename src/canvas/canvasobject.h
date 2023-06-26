#pragma once

#include "../shapes/shape.h"
#include "../shapes/shapeutils.h"
#include "../transforms/transformation.h"
#include "drawingvisitor.h"
#include "objectspace.h"

struct CanvasObject
{
    CanvasObject(const Shape &shape, Transformation transformation = {})
        : shape{shape}, boundingBox{ShapeUtils::CalculateBoundingBox(shape)}, transformation{transformation} {}

    void Draw(wxGraphicsContext &gc) const
    {
        gc.PushState();

        gc.SetTransform(gc.CreateMatrix(ObjectSpace::GetTransformationMatrix(*this)));
        std::visit(DrawingVisitor{gc}, shape);

        gc.PopState();
    }

    const Shape shape;
    const wxRect2DDouble boundingBox;
    Transformation transformation;
};
