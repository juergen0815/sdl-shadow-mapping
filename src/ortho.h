/*
 * ortho.h
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#ifndef ORTHO_H_
#define ORTHO_H_

#include "err.h"
#include "entity.h"

class Ortho : public Entity
{
private:
    // local render state
    struct RenderState : public ::RenderState
    {
        int     m_XPos;
        int     m_YPos;
        int     m_Width;
        int     m_Height;
        unsigned int m_ClearFlags;
        Vector  m_ClearColor;

        RenderState( int x, int y, int width, int height )
        : m_XPos(x)
        , m_YPos(y)
        , m_Width(width)
        , m_Height(height)
        , m_ClearFlags(0)
        , m_ClearColor(0.25f, 0.25f, 0.25f, 1.0f )
        {
        }

        void Set( int x, int y, int width, int height )
        {
            m_XPos  = x;
            m_YPos  = y;
            m_Width = width;
            m_Height= height;
        }
    } *m_RenderStateProxy;

public:
    Ortho( int x = 0, int y = 0, int width = 0, int height = 0 );

    virtual ~Ortho();

    void SetClearColor( const Vector& col );

    int GetWidth() const;

    int GetHeight() const;

    void Set( int x, int y, int width, int height );

    void SetSize( int width, int height );

    virtual void Render( int pass ) throw(std::exception);

    void SetClearFlags( unsigned int clearFlags ) { m_RenderStateProxy->m_ClearFlags = clearFlags; }

protected:
    virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

    virtual void DoRender( int pass ) throw(std::exception) {}
};

typedef boost::shared_ptr< Ortho > OrthoPtr;

#endif /* ORTHO_H_ */
