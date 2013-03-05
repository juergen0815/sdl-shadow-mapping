/*
 * light.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */


#include "light.h"
#include "renderer.h"

#include <GL/glew.h>

#include <boost/bind.hpp>

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
    int lighting(false);
    glGetIntegerv(GL_LIGHTING, &lighting );
    if (lighting) glEnable(GL_LIGHTING);

    // make us updateable - for moving lights
    renderer->RegisterUpdateFunction( boost::bind( &Light::Update, this, _1) );

    // currently this is static - if state changes, do again in DoUpdate

    glLightfv(m_RenderStateProxy->m_Index, GL_AMBIENT,  (const float*)m_RenderStateProxy->m_Ambient);
    glLightfv(m_RenderStateProxy->m_Index, GL_DIFFUSE,  (const float*)m_RenderStateProxy->m_Diffuse);
    glLightfv(m_RenderStateProxy->m_Index, GL_SPECULAR, (const float*)m_RenderStateProxy->m_Specular);
    glEnable(m_RenderStateProxy->m_Index);   // MUST enable each light source after configuration
    // copy position into vector - might need to overwrite spot light - need to transform as well
    m_RenderStateProxy->m_Position = &((const float*)m_RenderStateProxy->GetMatrix())[Matrix::POS_X];
    m_RenderStateProxy->m_Position[3] = 1.0f;
    glLightfv(m_RenderStateProxy->m_Index, GL_POSITION, (const float*)m_RenderStateProxy->m_Position );

    return true;
}

void Light::DoRender( int pass ) throw(std::exception)
{
//    glEnable(m_RenderStateProxy->m_Index);   // MUST enable each light source after configuration

    // transformed projectiopn matrix
    Matrix modelview;
    glGetFloatv( GL_MODELVIEW_MATRIX, (float*)modelview );

    // copy position into vector - might need to overwrite spot light - need to transform as well
//    m_RenderStateProxy->m_Position = &((const float*)modelview)[Matrix::POS_X];
//    glLightfv(GL_LIGHT0, GL_POSITION, (float*)m_RenderStateProxy->m_Position );
}

void Light::DoUpdate( float ticks ) throw(std::exception)
{
}
