/** OpenGLWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
#define FBCOMPOSITOR_XRENDERAUTOWINDOW_HH


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "OpenGLUtility.hh"
#include "OpenGLResources.hh"
#include "OpenGLTexPartitioner.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <vector>


namespace FbCompositor {

    class InitException;
    class OpenGLWindow;


    /**
     * Manages windows in OpenGL rendering mode.
     */
    class OpenGLWindow : public BaseCompWindow {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLWindow(const OpenGLScreen &screen, Window window_xid);

        /** Destructor. */
        virtual ~OpenGLWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the number of content partitions. */
        int partitionCount() const;


        /** \returns the specified content texture partiton. */
        const OpenGL2DTexturePtr contentTexturePartition(int id) const;

        /** \returns the specified shape texture partition. */
        const OpenGL2DTexturePtr shapeTexturePartition(int id) const;

        /** \returns the adjacent borders of the given partition. */
        unsigned int partitionBorders(int id) const;

        /** \returns the array buffer, containing the position of the given partition. */
        const OpenGLBufferPtr partitionPosBuffer(int id) const;


        /** \returns the window's screen, cast into the correct class. */
        const OpenGLScreen &openGLScreen() const;


        //--- WINDOW UPDATE FUNCTIONS ------------------------------------------

        /** Updates the window's contents. */
        void updateContents();

        /** Updates window's geometry. */
        void updateGeometry();

        /** Updates the window's shape. */
        void updateShape();

        /** Updates the window position vertex array. */
        void updateWindowPos();


    private :
        //--- RENDERING-RELATED VARIABLES --------------------------------------

        /** Window's content texture. */
        OpenGL2DTexturePartitionPtr m_content_tex_partition;

        /** Window's shape texture. */
        OpenGL2DTexturePartitionPtr m_shape_tex_partition;

        /** Window position buffer holder. */
        std::vector<OpenGLBufferPtr> m_window_pos_buffers;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the specified content texture partiton.
    inline const OpenGL2DTexturePtr OpenGLWindow::contentTexturePartition(int id) const {
        if ((id < 0) || (id >= partitionCount())) {
            throw IndexException("Out of bounds index in OpenGLWindow::contentTexturePartition.");
        }
        return m_content_tex_partition->partitions()[id].texture;
    }

    // Returns the adjacent borders of the given partition.
    inline unsigned int OpenGLWindow::partitionBorders(int id) const {
        if ((id < 0) || (id >= partitionCount())) {
            throw IndexException("Out of bounds index in OpenGLWindow::partitionBorders.");
        }
        return m_content_tex_partition->partitions()[id].borders;
    }

    // Returns the number of contents' partitions.
    inline int OpenGLWindow::partitionCount() const {
        return m_content_tex_partition->partitions().size();
    }

    // Returns the array buffer, containing the position of the given partition.
    inline const OpenGLBufferPtr OpenGLWindow::partitionPosBuffer(int id) const {
        if ((id < 0) || (id >= partitionCount())) {
            throw IndexException("Out of bounds index in OpenGLWindow::partitionPosBuffer.");
        }
        return m_window_pos_buffers[id];
    }

    // Returns the specified shape texture partition.
    inline const OpenGL2DTexturePtr OpenGLWindow::shapeTexturePartition(int id) const {
        if ((id < 0) || (id >= partitionCount())) {
            throw IndexException("Out of bounds index in OpenGLWindow::shapeTexturePartition.");
        }
        return m_shape_tex_partition->partitions()[id].texture;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOWINDOW_HH
