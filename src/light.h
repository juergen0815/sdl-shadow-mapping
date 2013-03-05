/*
 * light.h
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "err.h"
#include "entity.h"

class Light : public Entity
{
private:
    // local render state
    struct RenderState : public ::RenderState
    {
        int    m_Index;
        Vector m_Ambient;
        Vector m_Diffuse;
        Vector m_Specular;
        float  m_Shinyness;

        Vector m_Position;

    } *m_RenderStateProxy;

public:
    Light();

    virtual ~Light();
protected:
    virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

    virtual void DoRender( int pass ) throw(std::exception);

    virtual void DoUpdate( float ticks ) throw(std::exception);

};

typedef boost::shared_ptr< Light > LightPtr;

#endif /* LIGHT_H_ */
