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

}

bool Ortho::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    return true;
}
