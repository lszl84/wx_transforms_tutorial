#pragma once

#include <wx/docview.h>
#include <wx/stdstream.h>

#include "xmlserializer.h"
#include "canvas/canvasobject.h"

#include <iostream>

class DrawingDocument : public wxDocument
{
public:
    std::ostream &SaveObject(std::ostream &stream) override;
    std::istream &LoadObject(std::istream &stream) override;

    std::vector<CanvasObject> objects;
    XmlSerializer serializer;

    wxDECLARE_DYNAMIC_CLASS(DrawingDocument);
};