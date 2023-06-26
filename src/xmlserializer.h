#pragma once

#include <wx/xml/xml.h>
#include <wx/fs_zip.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#include <memory>

#include "shapes/shape.h"
#include "shapes/path.h"
#include "shapes/rect.h"
#include "shapes/circle.h"

namespace XmlNodeKeys
{
    constexpr auto ObjectNodeName = "Object";
    constexpr auto PathNodeType = "Path";
    constexpr auto RectNodeType = "Rect";
    constexpr auto CircleNodeType = "Circle";

    constexpr auto CenterElementNodeName = "Center";
    constexpr auto RectElementNodeName = "Rect";
    constexpr auto PointElementNodeName = "Point";

    constexpr auto ColorAttribute = "color";
    constexpr auto RadiusAttribute = "radius";
    constexpr auto XAttribute = "x";
    constexpr auto YAttribute = "y";
    constexpr auto WidthAttribute = "width";
    constexpr auto HeightAttribute = "height";
    constexpr auto TypeAttribute = "type";

    constexpr auto DocumentNodeName = "PaintDocument";
    constexpr auto VersionAttribute = "version";
    constexpr auto VersionValue = "1.2";
};

struct XmlSerializingVisitor
{
    XmlSerializingVisitor(wxXmlNode *parentNode) : parentNode(parentNode)
    {
    }

    wxXmlNode *parentNode;

    void operator()(const Circle &circle)
    {
        wxXmlNode *objectNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ObjectNodeName);
        objectNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::CircleNodeType);
        objectNode->AddAttribute(XmlNodeKeys::ColorAttribute, circle.color.GetAsString(wxC2S_HTML_SYNTAX));
        objectNode->AddAttribute(XmlNodeKeys::RadiusAttribute, wxString::FromDouble(circle.radius));

        wxXmlNode *circleNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::CenterElementNodeName);
        circleNode->AddAttribute(XmlNodeKeys::XAttribute, wxString::FromDouble(circle.center.m_x));
        circleNode->AddAttribute(XmlNodeKeys::YAttribute, wxString::FromDouble(circle.center.m_y));

        objectNode->AddChild(circleNode);
        parentNode->AddChild(objectNode);
    }

    void operator()(const Rect &rectangle)
    {
        wxXmlNode *objectNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ObjectNodeName);
        objectNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::RectNodeType);
        objectNode->AddAttribute(XmlNodeKeys::ColorAttribute, rectangle.color.GetAsString(wxC2S_HTML_SYNTAX));

        wxXmlNode *rectNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::RectElementNodeName);
        rectNode->AddAttribute(XmlNodeKeys::XAttribute, wxString::FromDouble(rectangle.rect.m_x));
        rectNode->AddAttribute(XmlNodeKeys::YAttribute, wxString::FromDouble(rectangle.rect.m_y));
        rectNode->AddAttribute(XmlNodeKeys::WidthAttribute, wxString::FromDouble(rectangle.rect.m_width));
        rectNode->AddAttribute(XmlNodeKeys::HeightAttribute, wxString::FromDouble(rectangle.rect.m_height));

        objectNode->AddChild(rectNode);
        parentNode->AddChild(objectNode);
    }

    void operator()(const Path &path)
    {
        wxXmlNode *objectNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::ObjectNodeName);

        objectNode->AddAttribute(XmlNodeKeys::TypeAttribute, XmlNodeKeys::PathNodeType);
        objectNode->AddAttribute(XmlNodeKeys::ColorAttribute, path.color.GetAsString(wxC2S_HTML_SYNTAX));
        objectNode->AddAttribute(XmlNodeKeys::WidthAttribute, wxString::FromDouble(path.width));

        for (const auto &point : path.points)
        {
            wxXmlNode *pointNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::PointElementNodeName);
            pointNode->AddAttribute(XmlNodeKeys::XAttribute, wxString::FromDouble(point.m_x));
            pointNode->AddAttribute(XmlNodeKeys::YAttribute, wxString::FromDouble(point.m_y));
            objectNode->AddChild(pointNode);
        }

        parentNode->AddChild(objectNode);
    }
};

struct XmlDeserializingObjectFactory
{
    Shape Deserialize(const wxXmlNode *node)
    {
        auto type = node->GetAttribute(XmlNodeKeys::TypeAttribute);
        if (type == XmlNodeKeys::PathNodeType)
        {
            return DeserializePath(node);
        }
        else if (type == XmlNodeKeys::RectNodeType)
        {
            return DeserializeRect(node);
        }
        else if (type == XmlNodeKeys::CircleNodeType)
        {
            return DeserializeCircle(node);
        }

        throw std::runtime_error("Unknown object type: " + type);
    }

private:
    Path DeserializePath(const wxXmlNode *node)
    {
        Path object{};
        object.color = wxColor(node->GetAttribute(XmlNodeKeys::ColorAttribute));
        object.width = wxAtof(node->GetAttribute(XmlNodeKeys::WidthAttribute));
        object.points = {};

        for (wxXmlNode *pointNode = node->GetChildren(); pointNode; pointNode = pointNode->GetNext())
        {
            if (pointNode->GetName() != XmlNodeKeys::PointElementNodeName)
                continue;

            object.points.push_back(wxPoint2DDouble(wxAtof(pointNode->GetAttribute(XmlNodeKeys::XAttribute)),
                                                    wxAtof(pointNode->GetAttribute(XmlNodeKeys::YAttribute))));
        }

        return object;
    }

    Circle DeserializeCircle(const wxXmlNode *node)
    {
        Circle object{};

        object.color.Set(node->GetAttribute(XmlNodeKeys::ColorAttribute));
        object.radius = wxAtof(node->GetAttribute(XmlNodeKeys::RadiusAttribute));

        const wxXmlNode *centerNode = node->GetChildren();
        object.center.m_x = wxAtof(centerNode->GetAttribute(XmlNodeKeys::XAttribute));
        object.center.m_y = wxAtof(centerNode->GetAttribute(XmlNodeKeys::YAttribute));

        return object;
    }

    Rect DeserializeRect(const wxXmlNode *node)
    {
        Rect object;
        object.color.Set(node->GetAttribute(XmlNodeKeys::ColorAttribute));

        const wxXmlNode *rectNode = node->GetChildren();
        object.rect.m_x = wxAtof(rectNode->GetAttribute(XmlNodeKeys::XAttribute));
        object.rect.m_y = wxAtof(rectNode->GetAttribute(XmlNodeKeys::YAttribute));
        object.rect.m_width = wxAtof(rectNode->GetAttribute(XmlNodeKeys::WidthAttribute));
        object.rect.m_height = wxAtof(rectNode->GetAttribute(XmlNodeKeys::HeightAttribute));

        return object;
    }
};

struct XmlSerializer
{
    XmlSerializer()
    {
        wxFileSystem::AddHandler(new wxZipFSHandler);
    }

    wxXmlDocument SerializeShapes(const std::vector<Shape> &objects)
    {
        wxXmlDocument doc;

        wxXmlNode *docNode = new wxXmlNode(wxXML_ELEMENT_NODE, XmlNodeKeys::DocumentNodeName);
        docNode->AddAttribute(XmlNodeKeys::VersionAttribute, XmlNodeKeys::VersionValue);

        XmlSerializingVisitor visitor{docNode};

        for (const auto &obj : objects)
        {
            std::visit(visitor, obj);
        }

        doc.SetRoot(docNode);

        return doc;
    }

    std::vector<Shape> DeserializeShapes(const wxXmlDocument &doc)
    {
        wxXmlNode *root = doc.GetRoot();

        std::vector<Shape> objects;

        XmlDeserializingObjectFactory factory{};

        for (wxXmlNode *node = root->GetChildren(); node; node = node->GetNext())
        {
            if (node->GetName() != XmlNodeKeys::ObjectNodeName)
                continue;

            objects.emplace_back(factory.Deserialize(node));
        }

        return objects;
    }

    void CompressXml(const wxXmlDocument &doc, wxOutputStream &outStream)
    {
        wxZipOutputStream zip(outStream);

        zip.PutNextEntry("paintdocument.xml");
        doc.Save(zip);

        zip.CloseEntry();

        zip.Close();
    }

    void CompressXml(const wxXmlDocument &doc, const wxString &zipFile)
    {
        auto outStream = wxFileOutputStream(zipFile);

        CompressXml(doc, outStream);
        outStream.Close();
    }

    wxXmlDocument DecompressXml(wxInputStream &in)
    {
        wxXmlDocument doc;
        wxZipInputStream zipIn(in);
        std::unique_ptr<wxZipEntry> entry(zipIn.GetNextEntry());

        while (entry)
        {
            wxString entryName = entry->GetName();

            if (entryName == "paintdocument.xml" && zipIn.CanRead())
            {
                doc.Load(zipIn);
                zipIn.CloseEntry();

                break;
            }

            zipIn.CloseEntry();
            entry.reset(zipIn.GetNextEntry());
        }

        return doc;
    }

    wxXmlDocument DecompressXml(const wxString &in)
    {
        wxFileSystem fs;
        std::unique_ptr<wxFSFile> zip(fs.OpenFile(in + "#zip:paintdocument.xml"));

        wxXmlDocument doc;

        if (zip)
        {
            wxInputStream *in = zip->GetStream();

            if (in)
            {
                doc.Load(*in);
            }
        }

        return doc;
    }
};