/*
 * light.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */


#include "light.h"
#include "renderer.h"

#include <GL/glew.h>

static int sLightId = GL_LIGHT0;

Light::Light()
    : m_RenderStateProxy( new RenderState )
{
    m_RenderStateProxy->m_Ambient = {.2f, .2f, .2f, 1.0f};  // ambient light
    m_RenderStateProxy->m_Diffuse = {.7f, .7f, .7f, 1.0f};  // diffuse light
    m_RenderStateProxy->m_Specular= {  1,   1,   1,    1};  // specular light

    m_RenderStateProxy->m_Index = sLightId; sLightId++;

    m_RenderState = RenderStatePtr( m_RenderStateProxy );
}

Light::~Light()
{

}

bool Light::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    // make us updateable - for moving lights
    renderer->RegisterUpdateFunction( boost::bind( &Light::Update, this, _1) );

    // currently this is static - if state changes, do again in DoUpdate

    glLightfv(m_RenderStateProxy->m_Index, GL_AMBIENT,  m_RenderStateProxy->m_Ambient);
    glLightfv(m_RenderStateProxy->m_Index, GL_DIFFUSE,  m_RenderStateProxy->m_Diffuse);
    glLightfv(m_RenderStateProxy->m_Index, GL_SPECULAR, m_RenderStateProxy->m_Specular);
    glEnable(m_RenderStateProxy->m_Index);   // MUST enable each light source after configuration

    return true;
}

void Light::DoRender( int pass ) throw(std::exception)
{
    // transformed projectiopn matrix
    Matrix projection;
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( projection );

    // copy position into vector - might need to overwrite spot light - need to transform as well
    m_RenderStateProxy->m_Position = &projection.m[Matrix::POS_X];
}

void Light::DoUpdate( float ticks ) throw(std::exception)
{
    glLightfv(GL_LIGHT0, GL_POSITION, (float*)m_RenderStateProxy->m_Position );
}
