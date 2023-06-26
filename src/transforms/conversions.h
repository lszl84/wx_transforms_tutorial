#pragma once

#include <wx/affinematrix2d.h>
#include "transformation.h"

namespace TransformWxConversions
{
    wxAffineMatrix2D GetMatrix(Transformation t, wxPoint2DDouble scaleCenter, wxPoint2DDouble rotationCenter)
    {
        wxAffineMatrix2D matrix;

        matrix.Translate(t.translationX, t.translationY);

        matrix.Translate(rotationCenter.m_x, rotationCenter.m_y);
        matrix.Rotate(t.rotationAngle);
        matrix.Translate(-rotationCenter.m_x, -rotationCenter.m_y);

        matrix.Translate(scaleCenter.m_x, scaleCenter.m_y);
        matrix.Scale(t.scaleX, t.scaleY);
        matrix.Translate(-scaleCenter.m_x, -scaleCenter.m_y);

        return matrix;
    }

    wxAffineMatrix2D GetInverseMatrix(Transformation t, wxPoint2DDouble scaleCenter, wxPoint2DDouble rotationCenter)
    {
        wxAffineMatrix2D matrix = GetMatrix(t, scaleCenter, rotationCenter);
        matrix.Invert();

        return matrix;
    }

}