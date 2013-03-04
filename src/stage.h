/*
 * stage.h
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */

#ifndef STAGE_H_
#define STAGE_H_

#include "err.h"
#include "entity.h"

class Stage : public Entity
{
public:
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

public:
    Stage();

    virtual ~Stage();

    virtual bool Initialize( Renderer* renderer ) throw(std::exception);

    virtual void Render( int pass ) throw(std::exception);

protected:
    virtual bool HandleEvent( const SDL_Event& event );

    virtual bool DoInitialize( Renderer* renderer ) throw( std::exception );

    virtual void DoRender( int pass ) throw( std::exception );

};


#endif /* STAGE_H_ */
