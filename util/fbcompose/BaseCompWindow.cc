/** BaseCompWindow.cc file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "BaseCompWindow.hh"

#include "Atoms.hh"
#include "BaseScreen.hh"
#include "Logging.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>

#include <algorithm>
#include <ostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseCompWindow::BaseCompWindow(const BaseScreen &screen, Window window_xid, bool track_damage_deltas) :
    FbTk::FbWindow(window_xid),
    m_screen(screen) {

    XWindowAttributes xwa;
    XGetWindowAttributes(display(), window(), &xwa);

    m_class = xwa.c_class;
    m_is_mapped = (xwa.map_state != IsUnmapped);
    m_is_remapped = true;
    m_is_resized = true;
    m_visual = xwa.visual;

    m_clip_shape_changed = true;
    m_clip_shape_rects = 0;
    m_clip_shape_rect_count = 0;

    m_is_ignored = false;

    if (m_class == InputOutput) {
        if (track_damage_deltas) {
            m_damage = XDamageCreate(display(), window_xid, XDamageReportDeltaRectangles);
        } else {
            m_damage = XDamageCreate(display(), window_xid, XDamageReportNonEmpty);
        }
    } else {
        m_damage = 0;
    }

    m_content_pixmap = None;

    updateAlpha();
    updateWindowType();

    XShapeSelectInput(display(), window_xid, ShapeNotifyMask);
}

// Destructor.
BaseCompWindow::~BaseCompWindow() {
    if (m_clip_shape_rects) {
        XFree(m_clip_shape_rects);
    }
    if (m_content_pixmap) {
        XFreePixmap(display(), m_content_pixmap);
    }
    // m_damage is apparently destroyed server-side.
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Add damage to a window.
void BaseCompWindow::addDamage() {
    m_is_damaged = true;
}

// Mark the window as mapped.
void BaseCompWindow::setMapped() {
    m_is_mapped = true;
    m_is_remapped = true;
}

// Mark the window as unmapped.
void BaseCompWindow::setUnmapped() {
    m_is_mapped = false;
}

// Update the window's contents.
// Note: this is an example implementation of this function. You should not
// call this version at any time and override it in derived classes.
void BaseCompWindow::updateContents() {
    updateContentPixmap();
    if (m_clip_shape_changed) {
        updateShape();
    }

    clearDamage();
}

// Update window's geometry.
void BaseCompWindow::updateGeometry() {
    unsigned int old_border_width = borderWidth();
    unsigned int old_height = height();
    unsigned int old_width = width();
    FbTk::FbWindow::updateGeometry();

    if ((borderWidth() != old_border_width) || (height() != old_height) || (width() != old_width)) {
        setClipShapeChanged();
        m_is_resized = true;
    }
}

// Update the window's clip shape.
void BaseCompWindow::updateShape() {
    int rect_order;

    if (m_clip_shape_rects) {
        XFree(m_clip_shape_rects);
        m_clip_shape_rects = NULL;
    }
    m_clip_shape_rects = XShapeGetRectangles(display(), window(), ShapeClip, &m_clip_shape_rect_count, &rect_order);

    if (!m_clip_shape_rects) {
        m_clip_shape_rect_count = 0;
    } else {
        // We have to adjust the size here to account for borders.
        for (int i = 0; i < m_clip_shape_rect_count; i++) {
            m_clip_shape_rects[i].height = std::min(m_clip_shape_rects[i].height + 2 * borderWidth(), realHeight());
            m_clip_shape_rects[i].width = std::min(m_clip_shape_rects[i].width + 2 * borderWidth(), realWidth());
        }
    }
}

// Update window's property.
void BaseCompWindow::updateProperty(Atom property, int /*state*/) {
    if (property == Atoms::opacityAtom()) {
        updateAlpha();
    } else if (property == Atoms::windowTypeAtom()) {
        updateWindowType();
    }
}


// Set the clip shape as changed.
void BaseCompWindow::setClipShapeChanged() {
    m_clip_shape_changed = true;
}


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Removes all damage from the window.
void BaseCompWindow::clearDamage() {
    m_clip_shape_changed = false;
    m_is_damaged = false;
    m_is_remapped = false;
    m_is_resized = false;
}

// Updates the window's content pixmap.
void BaseCompWindow::updateContentPixmap() {
    // We must reset the damage here, otherwise we may miss damage events.
    // TODO: BadDamage might be thrown here, probably safe to ignore for now.
    XDamageSubtract(display(), m_damage, None, None);

    if (m_is_resized || m_is_remapped) {
        XGrabServer(display());

        XWindowAttributes xwa;
        if (XGetWindowAttributes(display(), window(), &xwa)) {
            if (xwa.map_state == IsViewable) {
                Pixmap new_pixmap = XCompositeNameWindowPixmap(display(), window());
                if (new_pixmap) {
                    if (m_content_pixmap) {
                        XFreePixmap(display(), m_content_pixmap);
                    }
                    m_content_pixmap = new_pixmap;
                }
            }
        }
        XUngrabServer(display());
    }
}


//--- PROPERTY UPDATE FUNCTIONS ----------------------------------------

// Updates window's alpha.
void BaseCompWindow::updateAlpha() {
    m_alpha = singlePropertyValue<long>(Atoms::opacityAtom(), 0xff) & 0xff;
}

// Updates the type of the window.
void BaseCompWindow::updateWindowType() {
    static std::vector< std::pair<Atom, WindowType> > type_list = Atoms::windowTypeAtomList();
    Atom raw_type = singlePropertyValue<Atom>(Atoms::windowTypeAtom(), None);

    m_type = WinType_Normal;
    for (size_t i = 0; i < type_list.size(); i++) {
        if (raw_type == type_list[i].first) {
            m_type = type_list[i].second;
            break;
        }
    }
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Reads and returns raw property contents.
bool BaseCompWindow::rawPropertyData(Atom property_atom, Atom property_type,
                                     unsigned long *item_count_return, unsigned char **data_return) {
    Atom actual_type;
    int actual_format;
    unsigned long bytes_left;

    if (property(property_atom, 0, 0x7fffffff, False, property_type,
                 &actual_type, &actual_format, item_count_return, &bytes_left, data_return)) {
        if (*item_count_return > 0) {
            return true;
        }
    }

    return false;
}


//--- OPERATORS ----------------------------------------------------------------

// << output stream operator for the window class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseCompWindow& w) {
    out << "Window " << std::hex << w.window() << ": Geometry[" << std::dec << w.x()
        << "," << w.y() << "," << w.width() << "," << w.height() << " " << w.borderWidth()
        << "] Depth=" << w.depth() << " Type=" << w.type() << " Map=" << w.isMapped()
        << " Dmg=" << w.isDamaged() << " Ignore=" << w.isIgnored();
    return out;
}
