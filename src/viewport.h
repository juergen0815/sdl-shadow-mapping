/*
 * viewport.h
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include "err.h"
#include "entity.h"

#include <GL/glew.h>

struct Frustum
{
    float   m_Fov;
    float   m_ZNear,m_ZFar;

    float   m_Left;
    float   m_Right;
    float   m_Bottom;
    float   m_Top;
    Matrix  m_Matrix;

    Frustum( float fov = 45.0f, float zNear = 1.0f, float zFar = 100.0f );

    void Calculate( int w, int h );

    void Set();
};

class Viewport : public Entity
{
public:
private:
    // local render state
    struct RenderState : public ::RenderState
    {
        int     m_XPos;
        int     m_YPos;
        int     m_Width;
        int     m_Height;
        Vector  m_ClearColor;
        unsigned int m_ClearFlags;
        Frustum m_Frustum;

        RenderState( int x, int y, int width, int height )
        : m_XPos(x)
        , m_YPos(y)
        , m_Width(width)
        , m_Height(height)
        , m_ClearColor(0.25f, 0.25f, 0.25f, 1.0f )
        , m_ClearFlags( /* GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT */ )
        {
            m_Frustum.Calculate( width, height );
            // TODO:
        }

        void Set( int x, int y, int width, int height )
        {
            m_XPos  = x;
            m_YPos  = y;
            m_Width = width;
            m_Height= height;
            m_Frustum.Calculate( width, height );
        }

    } *m_RenderStateProxy;

public:
    Viewport( int x, int y, int width, int height );

    Viewport( int width = 0, int height = 0 );

    virtual ~Viewport();

    void SetClearColor( const Vector& col );

    void Reset( float fov, float zNear = 1.0f, float zFar = 100.0f );

    void Set( int x, int y, int width, int height );

    void SetSize( int width, int height );

    int GetWidth() const { return m_RenderStateProxy->m_Width; }

    int GetHeight() const { return m_RenderStateProxy->m_Height; }

    void SetClearFlags( unsigned int clearFlags ) { m_RenderStateProxy->m_ClearFlags = clearFlags; }

    virtual void Render( int pass ) throw(std::exception);
private:

    virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

    virtual void DoRender( int pass ) throw(std::exception) {}

    virtual void SetupRender( int pass );

    virtual void CleanupRender( int pass );
};

typedef boost::shared_ptr< Viewport > ViewportPtr;

#endif /* VIEWPORT_H_ */
