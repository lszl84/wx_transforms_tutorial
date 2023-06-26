#pragma once

#include <wx/stream.h>
#include <iostream>

struct OStreamWrapper : public wxOutputStream
{
    OStreamWrapper(std::ostream &stream) : m_stream(stream) {}

    size_t OnSysWrite(const void *buffer, size_t size) override
    {
        m_stream.write((const char *)buffer, size);
        return size;
    }

    std::ostream &m_stream;
};

struct IStreamWrapper : public wxInputStream
{
    IStreamWrapper(std::istream &stream) : m_stream(stream) {}

    size_t OnSysRead(void *buffer, size_t size) override
    {
        m_stream.read((char *)buffer, size);
        return size;
    }

    std::istream &m_stream;
};