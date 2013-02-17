/*
 * texture.cpp
 *
 *  Created on: Jan 19, 2013
 *      Author: Jurgen
 */

#include "texture.h"

#include <GL/glew.h>

Texture::Texture()
    : m_TextID( -1 )
    , m_Width(0)
    , m_Height(0)
    , m_TextureFilter(GL_LINEAR)
    , m_WrapMode(GL_REPEAT)
{
}

Texture::~Texture()
{
    if ( m_TextID > -1 ) {
        glDeleteTextures(1,(GLuint*)&m_TextID);
    }
}

void Texture::SetFilter( int filter )
{
    m_TextureFilter = filter;
    if ( m_TextID > -1 ) {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_TextureFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_TextureFilter );
    }
}

void Texture::SetWrapMode( int clampMode )
{
    m_WrapMode = clampMode;
    if ( m_TextID > -1 ) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);
    }
}

bool Texture::Allocate( int width, int height, int bpp /*= 4*/ ) throw(std::exception)
{
    bool r(false);
    m_Width  = width;
    m_Height = height;
    if ( m_TextID <= 0) {
        glGenTextures( 1, (GLuint*)&m_TextID );
    }
    GL_ASSERT( m_TextID > 0, "Error generating texture!" );

    glBindTexture( GL_TEXTURE_2D, m_TextID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_TextureFilter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_TextureFilter );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapMode);

    glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    GLenum fmt;
    GLenum ifmt;
    GLenum type(GL_UNSIGNED_BYTE);

    switch ( bpp )
    {
        // 0 is a depth map
    case 1: ifmt = fmt = GL_LUMINANCE; break; // could be GL_ALPHA - not supported. Use LUMINANCE_ALPHA
    case 2: ifmt = fmt = GL_LUMINANCE_ALPHA; break;
    case 3: ifmt = fmt = GL_RGB; break;
    case 4: ifmt = fmt = GL_RGBA; break;

    case 8:  ifmt = GL_DEPTH_COMPONENT;   fmt = GL_DEPTH_COMPONENT; type = GL_UNSIGNED_BYTE; break;
    case 16: ifmt = GL_DEPTH_COMPONENT16; fmt = GL_DEPTH_COMPONENT; type = GL_FLOAT; break;

    default: THROW( "Invalid Bytes per Pixel (%d). Must be 1, 2, 3 or 4.", bpp); break;
    }
    // generate pixel buffer in vmem
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, type, NULL);

    return true;
}

void Texture::Load( const char* pixels, int width, int height, int bpp, int type ) throw(std::exception)
{
    ASSERT( (bpp == 1 || bpp == 2 || bpp == 3 || bpp == 4), "Pixel data must be 1, 2, 3 or 4 bytes per pixel!" );
    if ( width > m_Width || height > m_Height ) {
        Allocate( width, height, bpp );
    }
    Enable();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, type, GL_UNSIGNED_BYTE, pixels);
}

void Texture::Load( const Brush& brush ) throw(std::exception)
{
    ASSERT( brush.m_Pixels && ( brush.m_BytesPerPixel == 3 || brush.m_BytesPerPixel == 4 ), "Invalid brush for texture!" );
    Load( brush.m_Pixels, brush.m_Width, brush.m_Height, brush.m_BytesPerPixel, brush.m_BytesPerPixel == 3 ? GL_BGR : GL_BGRA );
}

unsigned int Texture::GetTextureId() const
{
    return m_TextID;
}

void Texture::Bind() const
{
    ASSERT( m_TextID > -1, "Invalid Texture ID" );

    /* Typical Texture Generation Using Data From The Bitmap */
    glBindTexture( GL_TEXTURE_2D, m_TextID );
}

void Texture::Enable() const
{
    glEnable( GL_TEXTURE_2D );
    Bind();
}

void Texture::Disable() const
{
    glDisable( GL_TEXTURE_2D );
}

