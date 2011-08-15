// Layer.hh for Fluxbox Window Manager
// Copyright (c) 2006 Fluxbox Team (fluxgen at fluxbox dot org)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef RESOURCE_LAYER_HH
#define RESOURCE_LAYER_HH

#include "FbTk/StringUtil.hh"
#include "Resources.hh"

/** 
 * (This is not the layer->raise/lower handling stuff, @see FbTk::Layer)
 * Class to store layer numbers (special Resource type)
 * we have a special resource type because we need to be able to name certain layers
 * a Resource<int> wouldn't allow this
 */
class  ResourceLayer {
public:
    explicit ResourceLayer(int i) : m_num(i) {};

    static int getNumFromString(const std::string &str) {
        int tempnum = 0;
        std::string v = FbTk::StringUtil::toLower(str);
        if (FbTk::StringUtil::extractNumber(str, tempnum))
            return tempnum;
        if (v == "menu")
            return LAYERMENU;
        if (v == "abovedock")
            return LAYERABOVE_DOCK;
        if (v == "dock")
            return LAYERDOCK;
        if (v == "top")
            return LAYERTOP;
        if (v == "normal")
            return LAYERNORMAL;
        if (v == "bottom")
            return LAYERBOTTOM;
        if (v == "desktop")
            return LAYERDESKTOP;
        return -1;
    }

    static std::string getString(int num) {
        switch (num) {
        case LAYERMENU:
            return std::string("Menu");
        case LAYERABOVE_DOCK:
            return std::string("AboveDock");
        case LAYERDOCK:
            return std::string("Dock");
        case LAYERTOP:
            return std::string("Top");
        case LAYERNORMAL:
            return std::string("Normal");
        case LAYERBOTTOM:
            return std::string("Bottom");
        case LAYERDESKTOP:
            return std::string("Desktop");
        default:
           return FbTk::StringUtil::number2String(num);
        }
    }

    int getNum() const { return m_num; }
    std::string getString() const { return getString(m_num); }

    ResourceLayer &operator=(int num) { m_num = num; return *this; }

private:
    int m_num;
};

#endif // RESOURCE_LAYER_HH
