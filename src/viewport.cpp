
/*
 * viewport.cpp
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#include "viewport.h"

Frustum::Frustum( float fov /*= 60.0f */, float zNear /* = 1.0f */, float zFar /* = 100.0f */ )
    : m_Fov( fov )
    , m_ZNear( zNear )
    , m_ZFar( zFar )

    , m_Left(0)
    , m_Right(0)
    , m_Bottom(0)
    , m_Top(0)
{
}

void Frustum::Set()
{
    // TODO: Manually calculate frustum matrix - this here depends on the current gl context

    // Set projection frustum
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // generate frustum matrix
    glFrustum( m_Left, m_Right, m_Bottom, m_Top, m_ZNear, m_ZFar );
    // store in render-state projection matrix
    glGetFloatv( GL_PROJECTION_MATRIX, (float*)m_Matrix );
    glPopMatrix();
}

void Frustum::Calculate( int w, int h )
{
    static const float pi = 3.1415926535897932384626433832795f;

    if ( w > 0 && h > 0 ) {
        float aspect = float(w)/float(h);
        float fH = std::tan( m_Fov / 360.0f * pi ) * m_ZNear;
        float fW = fH * aspect;
        m_Left  = -fW;
        m_Right =  fW;
        m_Bottom= -fH;
        m_Top   =  fH;
    }
}

Viewport::Viewport( int width, int height )
    : m_RenderStateProxy( new RenderState( 0, 0, width, height ) )
{
    m_RenderState = RenderStatePtr(m_RenderStateProxy);
}

Viewport::Viewport( int x, int y, int width, int height )
    : m_RenderStateProxy( new RenderState( x, y, width, height ) )
{
    m_RenderState = RenderStatePtr(m_RenderStateProxy);
}

Viewport::~Viewport()
{
}

void Viewport::Reset( float fov, float zNear /*= 1.0f*/, float zFar /*= 100.0f*/ )
{
    m_RenderStateProxy->m_Frustum.m_Fov   = fov;
    m_RenderStateProxy->m_Frustum.m_ZNear = zNear;
    m_RenderStateProxy->m_Frustum.m_ZFar  = zFar;
    m_RenderStateProxy->m_Frustum.Calculate( GetWidth(), GetHeight() );
}

void Viewport::Set( int x, int y, int width, int height )
{
    m_RenderStateProxy->Set(x,y,width,height);
}

void Viewport::SetSize( int width, int height )
{
    m_RenderStateProxy->Set(m_RenderStateProxy->m_XPos,m_RenderStateProxy->m_YPos,width,height);
}

void Viewport::SetupRender( int pass )
{
    // Position from the top
    const SDL_VideoInfo* info = SDL_GetVideoInfo();
    float screenHeight(info->current_h);
    // make top the 0 for 2D positioning - must match ortho
    glViewport( m_RenderStateProxy->m_XPos, screenHeight - ( m_RenderStateProxy->m_YPos + m_RenderStateProxy->m_Height ),
                m_RenderStateProxy->m_Width, m_RenderStateProxy->m_Height );

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( (float*)m_RenderStateProxy->m_Frustum.m_Matrix );

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    // if regular mode do transform, if replay read & load projection matrix from render state
    glLoadMatrixf( GetRenderState()->GetMatrix() );

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
}

void Viewport::CleanupRender( int pass )
{
    // nothing here
}

void Viewport::Render( int pass ) throw(std::exception)
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
    Entity::Render( pass );

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
    // generate frustum matrix
    m_RenderStateProxy->m_Frustum.Set();

    glClearColor( m_RenderStateProxy->m_ClearColor[ Vector::R ],
                  m_RenderStateProxy->m_ClearColor[ Vector::G ],
                  m_RenderStateProxy->m_ClearColor[ Vector::B ],
                  m_RenderStateProxy->m_ClearColor[ Vector::A ]);

    return true;
}

void Viewport::SetClearColor( const Vector& col )
{
    m_RenderStateProxy->m_ClearColor = col;
}

