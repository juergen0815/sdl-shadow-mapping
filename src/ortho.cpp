/*
 * ortho.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#include "ortho.h"

#include "GL/glew.h"

Ortho::Ortho( int x, int y, int width, int height )
    : m_RenderStateProxy( new RenderState(x,y,width,height) )
{
}

Ortho::~Ortho()
{
}

void Ortho::SetClearColor( const Vector& col )
{
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

    int lighting(false);
    glGetIntegerv(GL_LIGHTING, &lighting );

    glDisable( GL_LIGHTING );
    // clear depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);

    Entity::Render( pass );

    if ( lighting ) glEnable( GL_LIGHTING );

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
