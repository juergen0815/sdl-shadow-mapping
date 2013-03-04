/*
 * world.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#include "world.h"

World::World()
    : m_IsInitialized(false)
{

}

World::~World()
{

}

void World::AddLight( LightPtr light )
{

}

bool World::DoInitialize( Renderer* renderer ) throw( std::exception )
{
    // we will connect this world to multiple view ports, but only initialize it once!
    if ( !m_IsInitialized ) {
        m_IsInitialized = true;

        // setup the scene
    }
    return true;
}

void World::DoRender( int pass ) throw( std::exception )
{

}


