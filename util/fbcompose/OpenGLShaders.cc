/** OpenGLShaders.cc file for the fluxbox compositor. */

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


#include "OpenGLShaders.hh"

#include "Exceptions.hh"
#include "Logging.hh"
#include "OpenGLPlugin.hh"

#include <sstream>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

/** Size of the info log buffer. */
const int INFO_LOG_BUFFER_SIZE = 256;


//--- VERTEX SHADER SOURCE -----------------------------------------------------

/** Head of the vertex shader source code. */
const GLchar VERTEX_SHADER_HEAD[] = "\
    #version 120                                                             \n\
                                                                             \n\
    attribute vec2 fb_InitMainTexCoord;                                      \n\
    attribute vec2 fb_InitPrimPos;                                           \n\
    attribute vec2 fb_InitShapeTexCoord;                                     \n\
                                                                             \n\
    varying vec2 fb_MainTexCoord;                                            \n\
    varying vec2 fb_ShapeTexCoord;                                           \n\
";

/** Middle of the vertex shader source code. */
const GLchar VERTEX_SHADER_MIDDLE[] = "\
    void main() {                                                            \n\
        gl_Position = vec4(fb_InitPrimPos, 0.0, 1.0);                        \n\
        fb_MainTexCoord = fb_InitMainTexCoord;                               \n\
        fb_ShapeTexCoord = fb_InitShapeTexCoord;                             \n\
";

/** Tail of the vertex shader source code. */
const GLchar VERTEX_SHADER_TAIL[] = "\
    }                                                                        \n\
";


//--- FRAGMENT SHADER SOURCE ---------------------------------------------------

/** Head of the fragment shader source code. */
const GLchar FRAGMENT_SHADER_HEAD[] = "\
    #version 120                                                             \n\
                                                                             \n\
    uniform float fb_Alpha;                                                  \n\
    uniform sampler2D fb_MainTexture;                                        \n\
    uniform sampler2D fb_ShapeTexture;                                       \n\
                                                                             \n\
    varying vec2 fb_MainTexCoord;                                            \n\
    varying vec2 fb_ShapeTexCoord;                                           \n\
";

/** Middle of the fragment shader source code. */
const GLchar FRAGMENT_SHADER_MIDDLE[] = "\
    void main() {                                                            \n\
        gl_FragColor = texture2D(fb_MainTexture, fb_MainTexCoord)            \n\
                       * texture2D(fb_ShapeTexture, fb_ShapeTexCoord)        \n\
                       * vec4(1.0, 1.0, 1.0, fb_Alpha);                      \n\
";

/** Tail of the fragment shader source code. */
const GLchar FRAGMENT_SHADER_TAIL[] = "\
    }                                                                        \n\
";


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLShaderProgram::OpenGLShaderProgram(const std::vector<BasePlugin*> &plugins) {
    std::stringstream ss;

    // Assemble vertex shader.
    ss.str("");
    ss << VERTEX_SHADER_HEAD;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << (dynamic_cast<OpenGLPlugin*>(plugins[i]))->vertexShader() << "\n";
    }
    ss << VERTEX_SHADER_MIDDLE;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << (dynamic_cast<OpenGLPlugin*>(plugins[i]))->pluginName() << "();\n";
    }
    ss << VERTEX_SHADER_TAIL;
    m_vertex_shader = createShader(GL_VERTEX_SHADER, ss.str().length(), ss.str().c_str());

    fbLog_debug << "Vertex shader source code:" << std::endl << ss.str() << std::endl;

    // Assemble fragment shader.
    ss.str("");
    ss << FRAGMENT_SHADER_HEAD;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << (dynamic_cast<OpenGLPlugin*>(plugins[i]))->fragmentShader() << "\n";
    }
    ss << FRAGMENT_SHADER_MIDDLE;
    for (size_t i = 0; i < plugins.size(); i++) {
        ss << (dynamic_cast<OpenGLPlugin*>(plugins[i]))->pluginName() << "();\n";
    }
    ss << FRAGMENT_SHADER_TAIL;
    m_fragment_shader = createShader(GL_FRAGMENT_SHADER, ss.str().length(), ss.str().c_str());

    fbLog_debug << "Fragment shader source code:" << std::endl << ss.str() << std::endl;

    // Create shader program.
    m_shader_program = createShaderProgram(m_vertex_shader, m_fragment_shader);

    // Initialize attribute locations.
    m_main_tex_coord_attrib = getAttributeLocation("fb_InitMainTexCoord");
    m_prim_pos_attrib = getAttributeLocation("fb_InitPrimPos");
    m_shape_tex_coord_attrib = getAttributeLocation("fb_InitShapeTexCoord");

    // Initialize uniform locations.
    m_alpha_uniform = getUniformLocation("fb_Alpha");
    m_main_tex_uniform = getUniformLocation("fb_MainTexture");
    m_shape_tex_uniform = getUniformLocation("fb_ShapeTexture");
}

// Destructor.
OpenGLShaderProgram::~OpenGLShaderProgram() {
    glDetachShader(m_shader_program, m_vertex_shader);
    glDetachShader(m_shader_program, m_fragment_shader);

    glDeleteProgram(m_shader_program);
    glDeleteShader(m_vertex_shader);
    glDeleteShader(m_fragment_shader);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Creates a shader.
GLuint OpenGLShaderProgram::createShader(GLenum shader_type, GLint source_length, const GLchar *source) {
    // Determine shader type.
    FbTk::FbString shaderName;
    if (shader_type == GL_VERTEX_SHADER) {
        shaderName = "vertex";
    } else if (shader_type == GL_GEOMETRY_SHADER) {  // For completeness.
        shaderName = "geometry";
    } else if (shader_type == GL_FRAGMENT_SHADER) {
        shaderName = "fragment";
    } else {
        throw InitException("createShader() was given an invalid shader type.");
    }

    // Create and compile.
    GLuint shader = glCreateShader(shader_type);
    if (!shader) {
        std::stringstream ss;
        ss << "Could not create " << shaderName << " shader.";
        throw InitException(ss.str());
    }

    glShaderSource(shader, 1, &source, &source_length);
    glCompileShader(shader);

    // Check for errors.
    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if (!compile_status) {
        GLsizei info_log_size;
        GLchar info_log[INFO_LOG_BUFFER_SIZE];
        glGetShaderInfoLog(shader, INFO_LOG_BUFFER_SIZE, &info_log_size, info_log);

        std::stringstream ss;
        ss << "Error in compilation of the " << shaderName << " shader: "
           << std::endl << (const char*)(info_log);
        throw InitException(ss.str());
    }

    return shader;
}

// Creates a shader program.
GLuint OpenGLShaderProgram::createShaderProgram(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    if (!program) {
        throw InitException("Cannot create a shader program.");
    }

    // Link program.
    if (vertex_shader) {
        glAttachShader(program, vertex_shader);
    }
    if (fragment_shader) {
        glAttachShader(program, fragment_shader);
    }
    glLinkProgram(program);

    // Check for errors.
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        GLsizei info_log_size;
        GLchar info_log[INFO_LOG_BUFFER_SIZE];
        glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, &info_log_size, info_log);

        std::stringstream ss;
        ss << "Error in linking of the shader program: " << std::endl << (const char*)(info_log);
        throw InitException(ss.str());
    }

    return program;
}
