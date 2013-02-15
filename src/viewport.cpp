
/*
 * viewport.cpp
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#include "viewport.h"

#include <GL/glew.h>

void Frustum::Set()
{
    glFrustum( m_Left, m_Right, m_Bottom, m_Top, m_ZNear, m_ZFar );
}

Viewport::Viewport( int width, int height )
    : m_Width(width)
    , m_Height(height)
{
    SetFrustum( width, height );
}

Viewport::~Viewport()
{
}

void Viewport::SetFrustum( int width, int height )
{
    const float pi = 3.1415926535897932384626433832795f;
    float fov = 60.0f;
    float aspect = float(m_Width)/float(m_Height);
    float zNear = 1.0f;
    float zFar = 1000.0f;
    float fH = std::tan( fov / 360.0f * pi ) * zNear;
    float fW = fH * aspect;
    m_Frustum = { -fW, fW, -fH, fH, zNear, zFar };
}

void Viewport::Render( int pass, float ticks ) throw(std::exception)
{
    // backup previous viewport - needed if we render into a pbuffer
    GLint vp[4];
    glGetIntegerv( GL_VIEWPORT, vp );
    // backup projection matrix for internal storage
    Matrix modelview;
    glGetFloatv( GL_MODELVIEW_MATRIX, (float*)modelview );
    Matrix projection;
    glGetFloatv( GL_PROJECTION_MATRIX, (float*)projection );

    // apply local viewport and render children
    Entity::Render( pass, ticks );

    // restore previous viewport
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    // restore projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( projection );
    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( modelview );
}

bool Viewport::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    // static frustum. Calc only once
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    m_Frustum.Set();
    glGetFloatv( GL_PROJECTION_MATRIX, (float*)m_Projection );
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return true;
}

bool Viewport::HandleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_VIDEORESIZE:
        m_Width  = event.resize.w;
        m_Height = event.resize.h;
        // TODO: Flag reset frustum
        SetFrustum( m_Width, m_Height );
        break;
    default: break;
    }
    return false;
}

void Viewport::SetClearColor( const Vector& col )
{
    m_ClearColor = col;
}

void Viewport::DoUpdate( float ticks ) throw(std::exception)
{
    // Only 3D we do not care about overlays - just yet
    glViewport(0, 0, (GLsizei)m_Width, (GLsizei)m_Height);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( m_Projection );

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // if we want to move this thing, we would need to multiply this here somehow...

    glClearColor( m_ClearColor[ Vector::R ],
                  m_ClearColor[ Vector::G ],
                  m_ClearColor[ Vector::B ],
                  m_ClearColor[ Vector::A ] );                   // background color
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}



