/*
 * framebuffer.cpp
 *
 *  Created on: 2013-01-24
 *      Author: jurgens
 */


#include "framebuffer.h"

#include <GL/glew.h>

FrameBuffer::FrameBuffer( uint32_t flags /* = F_ENABLE_DEPTH_BUFFER_F */ )
    : m_Flags(flags)
    , m_FrameBufferID(-1)
    , m_DepthBufferID(-1)
    , m_Texture( new Texture )
{
}

FrameBuffer::~FrameBuffer()
{
    // Probably not in d'tor - must be called from render thread
    if ( m_FrameBufferID > -1 ) {
        // release texture
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
        if ( m_DepthBufferID > -1 ) {
            // free depth buffer and attachment
            glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1,(GLuint*)&m_DepthBufferID);
        }
        // delete frame buffer
        glDeleteFramebuffers(1,(GLuint*)&m_DepthBufferID);
    }
}

TexturePtr FrameBuffer::GetTexture() const
{
    return m_Texture;
}

bool FrameBuffer::Allocate( int width, int height, int type /* = GL_RGBA */ )
{
    // TODO: Check for FBO extensions!

    bool b = glewGetExtension("GL_ARB_framebuffer_object");
    GL_ASSERT( b, "Missing extension 'ARB_framebuffer_object'!");

    glGenFramebuffers(1, (GLuint*)&m_FrameBufferID);
    GL_ASSERT( m_FrameBufferID > 0 , "Error generating frame buffer!" );
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

    // add a draw buffer - default = true
    if ( m_Flags & F_ENABLE_COLOR_BUFFER_F ) {

        // create a depth buffer
        if ( m_Flags & F_ENABLE_DEPTH_BUFFER_F ) {
            glGenRenderbuffers(1, (GLuint*)&m_DepthBufferID);
            GL_ASSERT( m_DepthBufferID > 0, "Error generating depth buffer!" );
            glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            // we will need to get access to this depth buffer - if we don't attach a texture
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferID);
        }

        // create a texture to render into
        b = m_Texture->Allocate( width, height, type );
        ASSERT( b, "Error allocating texture!" );
        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Texture->GetTextureId(), 0);
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
    }
    else if ( m_Flags & F_ENABLE_DEPTH_BUFFER_F ) {
        // depth buffer only
        // create a texture to render into
        b = m_Texture->Allocate( width, height, 16 ); // 16 bit float depth texture!
        ASSERT( b, "Error allocating texture!" );
        // Set "renderedTexture" as our depth attachement #0
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_Texture->GetTextureId(), 0);
        glDrawBuffer( NULL );
    }

    b &= ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE );
    return b;
}

void FrameBuffer::Enable()
{
    BOOST_ASSERT(m_FrameBufferID > -1);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
}

void FrameBuffer::Disable()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
