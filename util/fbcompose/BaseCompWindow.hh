/** BaseCompWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_WINDOW_HH
#define FBCOMPOSITOR_WINDOW_HH

#include "Enumerations.hh"

#include "FbTk/FbWindow.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xdamage.h>

#include <iosfwd>
#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class BaseCompWindow;


    //--- SUPPORTING FUNCTIONS -------------------------------------------------

    /** << output stream operator for the BaseCompWindow class. */
    std::ostream &operator<<(std::ostream& out, const BaseCompWindow& window);


    //--- BASE WINDOW CLASS ----------------------------------------------------

    /**
     * Base class for composited windows.
     */
    class BaseCompWindow : public FbTk::FbWindow {
        //--- FRIEND OPERATORS -------------------------------------------------

        friend std::ostream &operator<<(std::ostream& out, const BaseCompWindow& window);

    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BaseCompWindow(const BaseScreen &screen, Window window_xid, bool track_damage_deltas);

        /** Destructor. */
        virtual ~BaseCompWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the window's opacity. */
        int alpha() const;

        /** \returns the window's contents as a pixmap. */
        Pixmap contentPixmap() const;

        /** \returns whether the window is damaged or not. */
        bool isDamaged() const;

        /** \returns whether the window is ignored or not. */
        bool isIgnored() const;

        /** \returns whether the screen is mapped or not. */
        bool isMapped() const;

        /** \returns the window's screen. */
        const BaseScreen &screen() const;

        /** \returns the type of the window. */
        WindowType type() const;
        
        /** \returns the window's visual. */
        Visual *visual();

        /** \returns the window's visual (const version). */
        const Visual *visual() const;

        /** \returns the window's class. */
        int windowClass() const;


        /** \returns the window's dimensions as an XRectangle (borders factored in). */
        XRectangle dimensions() const;

        /** \returns the window's height with borders factored in. */
        unsigned int realHeight() const;

        /** \returns the window's width with borders factored in. */
        unsigned int realWidth() const;


        //--- PROPERTY ACCESS --------------------------------------------------

        /** \returns the value of the specified property. */
        template<class T>
        std::vector<T> propertyValue(Atom property_atom);

        /** Convenience function for accessing properties with a single value. */
        template<class T>
        T singlePropertyValue(Atom property_atom, T default_value);

        /** Convenience function that returns the first existing single atom value. */
        template<class T>
        T firstSinglePropertyValue(std::vector<Atom> property_atoms, T default_value);


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Add damage to a window. */
        virtual void addDamage();

        /** Mark the window as mapped. */
        virtual void setMapped();

        /** Mark the window as unmapped. */
        virtual void setUnmapped();

        /** Update the window's contents. */
        virtual void updateContents();

        /** Update window's geometry. */
        virtual void updateGeometry();

        /** Update window's property. */
        virtual void updateProperty(Atom property, int state);


        /** Set the clip shape as changed. */
        void setClipShapeChanged();

        /** Sets window's ignore flag. */
        void setIgnored(bool ignore_status);


    protected:
        //--- PROTECTED ACCESSORS ----------------------------------------------

        /** \returns whether the chip shape changed since the last update. */
        int clipShapeChanged() const;

        /** \returns the number of rectangles that make up the clip shape. */
        int clipShapeRectCount() const;

        /** \returns the rectangles that make up the clip shape. */
        XRectangle *clipShapeRects() const;


        /** \returns whether the window has been remapped since the last update. */
        bool isRemapped() const;

        /** \returns whether the window has been resized since the last update. */
        bool isResized() const;


        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Removes all damage from the window. */
        void clearDamage();

        /** Updates the window's content pixmap. */
        void updateContentPixmap();

        /** Update the window's clip shape. */
        virtual void updateShape();


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        BaseCompWindow(const BaseCompWindow&);

        /** Assignment operator. */
        BaseCompWindow &operator=(const BaseCompWindow&);


        //--- PROPERTY UPDATE FUNCTIONS ----------------------------------------

        /** Updates window's alpha. */
        void updateAlpha();

        /** Updates the type of the window. */
        void updateWindowType();


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Returns the raw contents of a property. */
        bool rawPropertyData(Atom property_atom, Atom property_type,
                             unsigned long *item_count_return, unsigned char **data_return);



        //--- WINDOW ATTRIBUTES ------------------------------------------------

        /** Window's screen. */
        const BaseScreen &m_screen;


        /** Window opacity. */
        int m_alpha;

        /** Window's class. */
        int m_class;

        /** Window's map state. */
        bool m_is_mapped;

        /** Window's type. */
        WindowType m_type;

        /** Window's visual. */
        Visual *m_visual;


        /** Window's content pixmap. */
        Pixmap m_content_pixmap;

        /** Window's damage object. */
        Damage m_damage;

        /** Shows whether the window is damaged. */
        bool m_is_damaged;

        /** Shows whether the window should be ignored by the renderers or not. */
        bool m_is_ignored;

        /** Shows whether the window has been remapped since the last update. */
        bool m_is_remapped;

        /** Shows whether the window has been resized since the last update. */
        bool m_is_resized;


        /** The number of rectangles of window's clip shape. */
        int m_clip_shape_rect_count;

        /** Rectangles, that make up the window's clip shape. */
        XRectangle *m_clip_shape_rects;

        /** Shows whether the clip shape changed since the last update. */
        bool m_clip_shape_changed;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's opacity.
    inline int BaseCompWindow::alpha() const {
        return m_alpha;
    }

    // Returns whether the chip shape changed since the last update.
    inline int BaseCompWindow::clipShapeChanged() const {
        return m_clip_shape_changed;
    }

    // Returns the number of rectangles that make up the clip shape.
    inline int BaseCompWindow::clipShapeRectCount() const {
        return m_clip_shape_rect_count;
    }

    // Returns the rectangles that make up the clip shape.
    inline XRectangle *BaseCompWindow::clipShapeRects() const {
        return m_clip_shape_rects;
    }

    // Returns the window's contents as a pixmap.
    inline Pixmap BaseCompWindow::contentPixmap() const {
        return m_content_pixmap;
    }

    // Returns the window's dimensions as an XRectangle (borders factored in).
    inline XRectangle BaseCompWindow::dimensions() const {
        XRectangle dim = { x(), y(), realWidth(), realHeight() };
        return dim;
    }

    // Returns whether the window is damaged or not.
    inline bool BaseCompWindow::isDamaged() const {
        return m_is_damaged;
    }

    // Returns whether the window is ignored or not.
    inline bool BaseCompWindow::isIgnored() const {
        return m_is_ignored;
    }

    // Returns whether the window is mapped or not.
    inline bool BaseCompWindow::isMapped() const {
        return m_is_mapped;
    }

    // Returns whether the window has been remapped since the last update.
    inline bool BaseCompWindow::isRemapped() const {
        return m_is_remapped;
    }

    // Returns whether the window has been resized since the last update.
    inline bool BaseCompWindow::isResized() const {
        return m_is_resized;
    }

    // Returns the window's screen.
    inline const BaseScreen &BaseCompWindow::screen() const {
        return m_screen;
    }

    // Sets the window's ignore flag.
    inline void BaseCompWindow::setIgnored(bool ignore_status) {
        m_is_ignored = ignore_status;
    }

    // Returns the window's height with borders factored in.
    inline unsigned int BaseCompWindow::realHeight() const {
        return height() + 2 * borderWidth();
    }

    // Returns the window's width with borders factored in.
    inline unsigned int BaseCompWindow::realWidth() const {
        return width() + 2 * borderWidth();
    }

    // Returns the type of the window.
    inline WindowType BaseCompWindow::type() const {
        return m_type;
    }

    // Returns the window's visual.
    inline Visual *BaseCompWindow::visual() {
        return m_visual;
    }

    // Returns the window's visual (const version).
    inline const Visual *BaseCompWindow::visual() const {
        return m_visual;
    }

    // Returns the window's class.
    inline int BaseCompWindow::windowClass() const {
        return m_class;
    }


    //--- PROPERTY ACCESS ----------------------------------------------------------

    // Returns the value of the specified property.
    template<class T>
    std::vector<T> BaseCompWindow::propertyValue(Atom property_atom) {
        if (property_atom) {
            unsigned long item_count;
            T *data;

            if (rawPropertyData(property_atom, AnyPropertyType, &item_count, reinterpret_cast<unsigned char**>(&data))) {
                std::vector<T> actualData(data, data + item_count);
                XFree(data);
                return actualData;
            }
        }

        return std::vector<T>();
    }

    // Convenience function for accessing properties with a single value.
    template<class T>
    T BaseCompWindow::singlePropertyValue(Atom property_atom, T default_value) {
        std::vector<T> values = propertyValue<T>(property_atom);
        if (values.size() == 0) {
            return default_value;
        } else {
            return values[0];
        }
    }

    // Convenience function that returns the first existing single atom value.
    template<class T>
    T BaseCompWindow::firstSinglePropertyValue(std::vector<Atom> property_atoms, T default_value) {
        for (size_t i = 0; i < property_atoms.size(); i++) {
            std::vector<T> values = propertyValue<T>(property_atoms[i]);
            if (values.size() != 0) {
                return values[0];
            }
        }
        return default_value;
    }
}

#endif  // FBCOMPOSITOR_WINDOW_HH
