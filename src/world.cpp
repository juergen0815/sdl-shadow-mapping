/*
 * world.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#include "world.h"
#include "surface.h"
#include "cube.h"
#include "sphere.h"
#include "cylinder.h"

World::World()
    : m_IsInitialized(false)
{

}

World::~World()
{

}

void World::AddLight( LightPtr light )
{
    m_Lights.push_back( light );
    // we still need to "render" and transform this light
    AddEntity( light );
}

const World::LightList& World::GetLights() const
{
    return m_Lights;
}

bool World::DoInitialize( Renderer* renderer ) throw( std::exception )
{
    // we will connect this world to multiple view ports, but only initialize it once!
    if ( !m_IsInitialized ) {
        m_IsInitialized = true;

        // setup the scene
        {
            EntityPtr e( new Cube( ) );
            e->GetRenderState()->Translate( Vector(-4.0, 3, 0), Vector(1.0f, 1.0f, 1.0f) );
            e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
            AddEntity(e, 20 );
        }
        {
            EntityPtr e( new Sphere( ) );
            e->GetRenderState()->Translate( Vector(-0.0, 2, 0), Vector(1.0f, 1.0f, 1.0f) );
            e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
            AddEntity(e, 20 );
        }
        {
            EntityPtr e( new Cylinder( ) );
            e->GetRenderState()->Translate( Vector(4.0, 2.5, 0), Vector(1.0f, 0.3f, 1.0f) );
            e->GetRenderState()->Rotate( Vector( 20.0f, -20.0f, 0.0f ) );
            AddEntity(e, 20 );
        }

    }
    return true;
}

void World::DoRender( int pass ) throw( std::exception )
{

}


