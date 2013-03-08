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
#include "light.h"

#include <list>

class World : public Entity
{
public:
    typedef std::list< LightPtr > LightList;
private:
    bool        m_IsInitialized;
    LightList   m_Lights;
public:
    World();

    virtual ~World();

    void AddLight( LightPtr light );

    virtual bool Initialize( Renderer* renderer ) throw(std::exception);

    const LightList& GetLights() const;
protected:
    virtual bool DoInitialize( Renderer* renderer ) throw( std::exception );

    virtual void DoRender( int pass ) throw( std::exception );

    virtual void RenderSubTree( int pass ) throw( std::exception );

    virtual void SetupRender( int pass );

    virtual void CleanupRender( int pass );
};

typedef boost::shared_ptr<World> WorldPtr;

#endif /* WORLD_H_ */
