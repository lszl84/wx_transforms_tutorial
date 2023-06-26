#pragma once

#include <wx/docview.h>
#include <wx/stdstream.h>

#include "xmlserializer.h"
#include "shapes/shape.h"

#include <iostream>

class DrawingDocument : public wxDocument
{
public:
    std::ostream &SaveObject(std::ostream &stream) override;
    std::istream &LoadObject(std::istream &stream) override;

    std::vector<Shape> shapes;
    XmlSerializer serializer;

    wxDECLARE_DYNAMIC_CLASS(DrawingDocument);
};