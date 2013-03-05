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

#include "viewport.h"
#include "ortho.h"
#include "camera.h"
#include "world.h"
#include "framebuffer.h"

class Stage : public Entity
{
public:

private:
    OrthoPtr    m_Overlay;          // a generic overlay (hud or what ever)

    // That's our stage. One main, and 2 side stages
    // +----------+---+
    // |1 Main    | 2 | ShadowMap
    // |          +---+
    // |          | 3 | Lighting only
    // +----------+---+

    ViewportPtr m_MainStage;        // main stage - the combined image incl. shadow
    OrthoPtr    m_ShadowMapStage;   // a viewport to display the shado map texture (as an grey scale image)
    ViewportPtr m_LightingStage;    // default lit side stage - no shadows

    CameraPtr   m_Camera;           // Once camera for main stage
    WorldPtr    m_World;            // this is the world we want to render

    FrameBuffer m_ShadowMap;        // we render our "world" into a offscreen depth buffer
public:
    Stage( int w, int h, SDL_Joystick* joystick );

    virtual ~Stage();

    virtual bool Initialize( Renderer* renderer ) throw(std::exception);

    virtual void Render( int pass ) throw(std::exception);

protected:
    virtual bool HandleEvent( const SDL_Event& event );

    virtual bool DoInitialize( Renderer* renderer ) throw( std::exception );

    virtual void DoRender( int pass ) throw( std::exception );

    void OnResize( int w, int h );
};


#endif /* STAGE_H_ */
