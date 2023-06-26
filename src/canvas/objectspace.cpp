#include <wx/geometry.h>
#include <wx/affinematrix2d.h>

#include "objectspace.h"
#include "canvasobject.h"
#include "../transforms/conversions.h"

namespace ObjectSpace
{
    wxPoint2DDouble ToObjectCoordinates(const CanvasObject &object, wxPoint2DDouble point)
    {
        return GetInverseTransformationMatrix(object).TransformPoint(point);
    }

    wxPoint2DDouble ToObjectDistance(const CanvasObject &object, wxPoint2DDouble point)
    {
        return GetInverseTransformationMatrix(object).TransformDistance(point);
    }

    wxPoint2DDouble ToScreenCoordinates(const CanvasObject &object, wxPoint2DDouble point)
    {
        return GetTransformationMatrix(object).TransformPoint(point);
    }

    wxPoint2DDouble ToScreenDistance(const CanvasObject &object, wxPoint2DDouble point)
    {
        return GetTransformationMatrix(object).TransformDistance(point);
    }

    wxAffineMatrix2D GetTransformationMatrix(const CanvasObject &object)
    {
        return TransformWxConversions::GetMatrix(object.transformation, object.boundingBox.GetCentre(), object.boundingBox.GetCentre());
    }

    wxAffineMatrix2D GetInverseTransformationMatrix(const CanvasObject &object)
    {
        return TransformWxConversions::GetInverseMatrix(object.transformation, object.boundingBox.GetCentre(), object.boundingBox.GetCentre());
    }
}