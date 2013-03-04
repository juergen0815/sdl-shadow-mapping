/*
 * world.h
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "err.h"
#include "entity.h"

class World : public Entity
{
    bool m_IsInitialized;
public:
    World();

    virtual ~World();

    void AddLight( LightPtr light );

protected:
    virtual bool DoInitialize( Renderer* renderer ) throw( std::exception );

    virtual void DoRender( int pass ) throw( std::exception );

};

typedef boost::shared_ptr<World> WorldPtr;

#endif /* WORLD_H_ */
