/*
 * entity.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef ENTITY_H_
#define ENTITY_H_

#include "vector.h"
#include "renderstate.h"

#include <SDL/SDL_events.h>

#include <boost/shared_ptr.hpp>

#include <list>

class Renderer;

class Entity;
typedef boost::shared_ptr< Entity > EntityPtr;

typedef std::list< EntityPtr > EntityList;

class Entity
{
    enum enFLAG_BITS {
        F_ENABLE_B = 0,
        F_VISIBLE_B,
        F_DELETE_B,
    };
public:
    enum enFLAG
    {
        F_ENABLE  = (1<<F_ENABLE_B),
        F_VISIBLE = (1<<F_VISIBLE_B),
        F_DELETE  = (1<<F_DELETE_B),
    };

    enum enRENDER_PASS {
        PASS_SHADOW_MAP = 0, // render shadow map
        PASS_LIGHTING   = 1, // render regular pass (incl. lighting)
        PASS_SHADOW_TEST= 2, // combine shadow map

        NUM_PASSES
    };
    enum enRENDER_PASS_F {
        PASS_SHADOW_MAP_F = 1<<PASS_SHADOW_MAP,  // render shadow map
        PASS_LIGHTING_F   = 1<<PASS_LIGHTING,    // render regular pass (incl. lighting)
        PASS_SHADOW_TEST_F= 1<<PASS_SHADOW_TEST, // combine shadow map
    };
private:
    uint32_t m_Flags;
    int      m_OrderNum;

    RenderStatePtr m_RenderState;
protected:
    EntityList  m_RenderList;
    EntityList  m_InitList;
public:
    Entity() throw ();

	virtual ~Entity() {}

	virtual bool HandleEvent( const SDL_Event& event )  { return false; }

	uint32_t GetFlags() const { return m_Flags; }

	bool IsFlagSet( unsigned int flags ) const { return (m_Flags & flags) != 0; }

    void SetFlag( enFLAG flag ) { m_Flags |= flag; }

    void ClearFlag( enFLAG flag ) { m_Flags &= ~flag; }

    RenderStatePtr GetRenderState();

    virtual bool Initialize( Renderer* renderer ) throw(std::exception);

    virtual void Render( int pass ) throw(std::exception);

    virtual void CheckDestroy( ) throw(std::exception);

    /*!
     * This is not called unless explicitly registered to the renderer. It also
     * does not propagate the render tree.
     * This allows to flexible with the update methods and register external updaters
     */
    bool Update( float ticks ) throw(std::exception);

    void AddEntity( EntityPtr entity, int priority = 0 );

	// Only renderer has access to these below
private:
    void SetOrder( int order ) { m_OrderNum = order; }

    int GetOrder() const { return m_OrderNum; };

    bool CompareEntityPriorities( const EntityPtr& a, const EntityPtr& b );
protected:
    virtual bool DoInitialize( Renderer* renderer ) throw( std::exception ) = 0;

    virtual void DoRender( int pass ) throw( std::exception ) = 0;

    virtual void DoUpdate( float ticks ) throw(std::exception) {};

    friend class Renderer;
};

#endif /* ENTITY_H_ */
