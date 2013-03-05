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
    LightPtr light( new Light );
    light->GetRenderState()->Translate( Vector( 0, 5, 0 ) );
//    AddLight( light );

    // setup the scene
    {
        EntityPtr e( new Cube( ) );
        e->GetRenderState()->Translate( Vector(-4.0, 1, 0), Vector(1.0f, 1.0f, 1.0f) );
        e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
        AddEntity(e, 20 );
    }
    {
        EntityPtr e( new Sphere( ) );
        e->GetRenderState()->Translate( Vector(-0.0, 0, 0), Vector(1.0f, 1.0f, 1.0f) );
        e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
        AddEntity(e, 20 );
    }
    {
        EntityPtr e( new Cylinder( ) );
        e->GetRenderState()->Translate( Vector(4.0, 0.5, 0), Vector(1.0f, 0.3f, 1.0f) );
        e->GetRenderState()->Rotate( Vector( 20.0f, -20.0f, 0.0f ) );
        AddEntity(e, 20 );
    }
}

World::~World()
{
}

void World::AddLight( LightPtr light )
{
    m_Lights.push_back( light );
    AddEntity( light, 0 );
}

const World::LightList& World::GetLights() const
{
    return m_Lights;
}

bool World::Initialize( Renderer* renderer ) throw(std::exception)
{
    bool r(false);
    // we will connect this world to multiple view ports, but only initialize it once!
    if ( !m_IsInitialized ) {
        m_IsInitialized = true;
        r = Entity::Initialize(renderer);
    }
    return r;
}

bool World::DoInitialize( Renderer* renderer ) throw( std::exception )
{
    bool r(true);
    // transform/render all lights
    for ( auto& light : m_Lights ) {
//        r &= light->Initialize( renderer );
    }
    return r;
}

void World::RenderSubTree( int pass ) throw( std::exception )
{
    // no more transformation, etc.

    // Transform/Render lights before children
    DoRender( pass );

    // render all children
    for( auto it = m_RenderList.begin(); it != m_RenderList.end(); ) {
        EntityPtr entity = *it;
        if ( entity->IsFlagSet( Entity::F_ENABLE|Entity::F_VISIBLE ) &&
            !entity->IsFlagSet( Entity::F_DELETE ) )
        {
            // don't bother rendering if we are marked for deletion
            entity->Render( pass );
        }
        ++it;
    }
}

void World::DoRender( int pass ) throw( std::exception )
{
    // transform/render all lights
    for ( auto& light : m_Lights ) {
//        light->Render( pass );
    }
}


