#include "drawingdocument.h"
#include "utils/streamutils.h"

wxIMPLEMENT_DYNAMIC_CLASS(DrawingDocument, wxDocument);

std::ostream &DrawingDocument::SaveObject(std::ostream &stream)
{
    auto doc = serializer.SerializeShapes(shapes);

    auto wrapper = OStreamWrapper(stream);
    serializer.CompressXml(doc, wrapper);
    return stream;
}

std::istream &DrawingDocument::LoadObject(std::istream &stream)
{
    auto wrapper = IStreamWrapper(stream);
    auto doc = serializer.DecompressXml(wrapper);

    shapes = serializer.DeserializeShapes(doc);

    // workaround for wxWidgets problem: https://github.com/wxWidgets/wxWidgets/issues/23479
    stream.clear();

    return stream;
}