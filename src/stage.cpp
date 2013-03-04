/*
 * stage.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */


#include "stage.h"

    Stage::Stage()
    {

    }

    Stage::~Stage()
    {

    }

    bool Stage::Initialize( Renderer* renderer ) throw(std::exception)
    {
        bool r = Entity::Initialize( renderer );
        return r;
    }

    void Stage::Render( int pass ) throw(std::exception)
    {
        // use a lookup table so we can actually reorder these if needed, or add new, or w/e
        int renderPasses[ NUM_PASSES ] = { PASS_SHADOW_MAP, PASS_LIGHTING, PASS_SHADOW_TEST };
        int renderPass(0);
        do {
            int p = renderPasses[renderPass];
            switch ( p )
            {
            case PASS_SHADOW_MAP:
                // render from light pos into depth map
                break;
            case PASS_LIGHTING:
            default:
                // default lighting pass - render whole scene
                Entity::Render( 1<<p );
                break;
            case PASS_SHADOW_TEST:
                // render shadows from shadow map into scene
                break;
            }
        } while (++renderPass < NUM_PASSES );
    }

    bool Stage::HandleEvent( const SDL_Event& event )
    {
        bool eventHandled(false);
        switch (event.type) {
        case SDL_VIDEORESIZE:
            // TODO: resize our viewports

            break;
        default: break;
        }
        return eventHandled;
    }

    bool Stage::DoInitialize( Renderer* renderer ) throw( std::exception )
    {

        return true;
    }

    void Stage::DoRender( int pass ) throw( std::exception )
    {
        // not really much to do here. This would be used if the stage would want to render something (like, hub, or info or w/e)
    }

