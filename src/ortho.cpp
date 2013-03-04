/*
 * ortho.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#include "ortho.h"

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


void Ortho::Render( int pass ) throw(std::exception)
{
}

