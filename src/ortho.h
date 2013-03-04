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
        Vector  m_ClearColor;

        RenderState( int x, int y, int width, int height )
        : m_XPos(x)
        , m_YPos(y)
        , m_Width(width)
        , m_Height(height)
        , m_ClearColor(0.25f, 0.25f, 0.25f, 1.0f )
        {
        }
    } *m_RenderStateProxy;

public:
    Ortho( int x, int y, int width, int height );

    virtual ~Ortho();

    void SetClearColor( const Vector& col );

    int GetWidth() const;

    int GetHeight() const;

    virtual void Render( int pass ) throw(std::exception);

};

#endif /* ORTHO_H_ */
