/** OpenGLTexPartitioner.cc file for the fluxbox compositor. */

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


#include "OpenGLTexPartitioner.hh"

#include "OpenGLScreen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGL2DTexturePartition::OpenGL2DTexturePartition(const OpenGLScreen &screen, bool swizzle_alpha_to_one) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_max_texture_size = screen.maxTextureSize();
    m_pixmap = None;
    m_swizzle_alpha_to_one = swizzle_alpha_to_one;

    m_full_height = 0;
    m_full_width = 0;
}

// Destructor.
OpenGL2DTexturePartition::~OpenGL2DTexturePartition() { }


//--- MUTATORS -----------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGL2DTexturePartition::setPixmap(Pixmap pixmap, bool manage_pixmap, int width, int height, int depth) {
    // Handle the pixmap and its GC.
    if (m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
        m_pixmap = None;
    }
    if (manage_pixmap) {
        m_pixmap = pixmap;
    }
    GC gc = XCreateGC(m_display, pixmap, 0, NULL);

    // Set partition's dimensions and partition that space.
    m_full_height = height;
    m_full_width = width;

    int unit_height, unit_width;
    std::vector<XRectangle> space_parts = partitionSpace(0, 0, width, height, m_max_texture_size, &unit_width, &unit_height);
    int total_units = unit_height * unit_width;

    // Adjust number of stored partitions.
    while ((size_t)(total_units) > m_partitions.size()) {
        TexturePart partition;
        partition.borders = 0;
        partition.texture = new OpenGL2DTexture(m_screen, m_swizzle_alpha_to_one);
        m_partitions.push_back(partition);
    }
    while ((size_t)(total_units) < m_partitions.size()) {
        m_partitions.pop_back();
    }

    // Create texture partitions.
    if (total_units == 1) {
        m_partitions[0].borders = BORDER_ALL;
        m_partitions[0].texture->setPixmap(pixmap, false, m_full_width, m_full_height, false);
    } else {
        for (int i = 0; i < unit_height; i++) {
            for (int j = 0; j < unit_width; j++) {
                int idx = i * unit_width + j;

                // Create partition's pixmap.
                Pixmap part_pixmap = XCreatePixmap(m_display, m_screen.rootWindow().window(),
                                                   space_parts[idx].width, space_parts[idx].height, depth);
                XCopyArea(m_display, pixmap, part_pixmap, gc, space_parts[idx].x, space_parts[idx].y,
                          space_parts[idx].width, space_parts[idx].height, 0, 0);

                // Set up the partition.
                m_partitions[idx].borders = getBorderBitfield(unit_width, unit_height, j, i);
                m_partitions[idx].texture->setPixmap(part_pixmap, true, space_parts[idx].width, space_parts[idx].height, false);
            }
        }
    }

    // Cleanup.
    XFreeGC(m_display, gc);
}


//--- SUPPORTING FUNCTIONS -------------------------------------------------

// Returns the border bitfield of the given partition.
unsigned int FbCompositor::getBorderBitfield(int unit_width, int unit_height, int x, int y) {
    unsigned int borders = 0;
    borders |= ((y == 0) ? BORDER_NORTH : 0);
    borders |= ((x == 0) ? BORDER_WEST : 0);
    borders |= ((y == (unit_height - 1)) ? BORDER_SOUTH : 0);
    borders |= ((x == (unit_width - 1)) ? BORDER_EAST : 0);
    return borders;
}

// Space partitioning function.
std::vector<XRectangle> FbCompositor::partitionSpace(int x, int y, int width, int height, int max_partition_size,
                                                     int *unit_width_return, int *unit_height_return) {
    int unit_height = ((height - 1) / max_partition_size) + 1;
    int unit_width = ((width - 1) / max_partition_size) + 1;

    std::vector<XRectangle> partitions;
    for (int i = 0; i < unit_height; i++) {
        for (int j = 0; j < unit_width; j++) {
            int part_x = x + j * max_partition_size;
            int part_y = y + i * max_partition_size;
            int part_height = ((i == (unit_height - 1)) ? (height % max_partition_size) : max_partition_size);
            int part_width = ((j == (unit_width - 1)) ? (width % max_partition_size) : max_partition_size);

            XRectangle part = { part_x, part_y, part_width, part_height };
            partitions.push_back(part);
        }
    }

    if (unit_width_return && unit_height_return) {
        *unit_width_return = unit_width;
        *unit_height_return = unit_height;
    }

    return partitions;
}

// Partitions space directly to buffers.
std::vector<OpenGLBufferPtr> FbCompositor::partitionSpaceToBuffers(const OpenGLScreen &screen,
        int x, int y, int width, int height) {

    std::vector<XRectangle> space_part = partitionSpace(x, y, width, height, screen.maxTextureSize());
    std::vector<OpenGLBufferPtr> buffers;

    for (size_t i = 0; i < space_part.size(); i++) {
        OpenGLBufferPtr buffer(new OpenGLBuffer(screen, GL_ARRAY_BUFFER));
        buffer->bufferPosRectangle(screen.rootWindow().width(), screen.rootWindow().height(), space_part[i]);
        buffers.push_back(buffer);
    }

    return buffers;
}
