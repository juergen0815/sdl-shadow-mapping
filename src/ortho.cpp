/*
 * ortho.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#include "ortho.h"

#include "GL/glew.h"

#include <SDL/SDL.h>

Ortho::Ortho( int x, int y, int width, int height )
    : m_RenderStateProxy( new RenderState(x,y,width,height) )
{
}

Ortho::~Ortho()
{
}

void Ortho::SetClearColor( const Vector& col )
{
    m_RenderStateProxy->m_ClearColor = col;
}

int Ortho::GetWidth() const
{
    return m_RenderStateProxy->m_Width;
}

int Ortho::GetHeight() const
{
    return m_RenderStateProxy->m_Height;
}

void Ortho::Set( int x, int y, int width, int height )
{
    m_RenderStateProxy->Set(x,y,width,height);
}

void Ortho::SetSize( int width, int height )
{
    m_RenderStateProxy->Set(m_RenderStateProxy->m_XPos,m_RenderStateProxy->m_YPos,width,height);
}

void Ortho::Render( int pass ) throw(std::exception)
{
    // backup previous viewport - needed if we render into a pbuffer
    GLint vp[4];
    glGetIntegerv( GL_VIEWPORT, vp );
    // backup projection matrix for internal storage
    Matrix modelview;
    glGetFloatv( GL_MODELVIEW_MATRIX, (float*)modelview );

    Matrix projection;
    glGetFloatv( GL_PROJECTION_MATRIX, (float*)projection );

    int lighting;
    glGetIntegerv(GL_LIGHTING, &lighting );
    if ( lighting ) glDisable( GL_LIGHTING );

    int depthTest;
    glGetIntegerv(GL_DEPTH_TEST, &depthTest );
    if ( depthTest ) glDisable( GL_DEPTH_TEST );

    // Position from the top
    const SDL_VideoInfo* info = SDL_GetVideoInfo();
    float screenHeight(info->current_h);
    // need to get this from the screen
    glViewport( m_RenderStateProxy->m_XPos, screenHeight - ( m_RenderStateProxy->m_YPos + m_RenderStateProxy->m_Height ),
    		    m_RenderStateProxy->m_Width, m_RenderStateProxy->m_Height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    // flip this upside down
//    float x0(m_RenderStateProxy->m_XPos);
//    float x1(m_RenderStateProxy->m_XPos+m_RenderStateProxy->m_Width-1);
//    float y0(m_RenderStateProxy->m_YPos+m_RenderStateProxy->m_Height-1);
//    float y1(m_RenderStateProxy->m_YPos);
//    glOrtho( x0, x1, y0, y1, -1000.0f, 1000.0f );
    float w(5);
    float h = w * float(m_RenderStateProxy->m_Width)/float(m_RenderStateProxy->m_Height);
    glOrtho( -w, w, -h, h, -100.0f, 100.0f );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // if regular mode do transform, if replay read & load projection matrix from render state
    glMultMatrixf( GetRenderState()->GetMatrix() );

    // use a flag to enable clear color / and clear flags
    if ( m_RenderStateProxy->m_ClearFlags ) {
        if ( m_RenderStateProxy->m_ClearFlags & GL_COLOR_BUFFER_BIT ) {
            glClearColor( m_RenderStateProxy->m_ClearColor[ Vector::R ],
                          m_RenderStateProxy->m_ClearColor[ Vector::G ],
                          m_RenderStateProxy->m_ClearColor[ Vector::B ],
                          m_RenderStateProxy->m_ClearColor[ Vector::A ]);
        }
        // clear buffer
        glClear( m_RenderStateProxy->m_ClearFlags );
    }

    Entity::Render( pass );

    if ( lighting )  glEnable( GL_LIGHTING );
    if ( depthTest ) glEnable( GL_DEPTH_TEST );

    // restore previous viewport
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    // restore projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( projection );
    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( modelview );
}

bool Ortho::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    return true;
}
