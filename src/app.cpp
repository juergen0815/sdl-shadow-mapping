/*
 * app.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "err.h"
#include "app.h"
#include "renderer.h"
#include "stage.h"
#include "brushloader.h"

#include "world.h"
#include "surface.h"
#include "cube.h"
#include "sphere.h"
#include "cylinder.h"
#include "camera.h"

#include "viewport.h"
#include "ortho.h"

#include <SDL/SDL.h>

#include <GL/glew.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include <string>

App::App()
    : m_Worker(new Renderer)
    , m_Joystick(nullptr)
{
}

App::~App()
{
    if ( m_Joystick ) SDL_JoystickClose(m_Joystick);
    // probably a good idea to check if we have actually been created
    SDL_Quit();
}

// example of generic event handler
bool OnHandleEvent( const SDL_Event& event, EntityPtr c1, EntityPtr c2, EntityPtr c3  )
{
    ASSERT( c1 , "No entities in event handler!" );

    bool processed(false);
    switch (event.type)
    {
    case SDL_VIDEORESIZE: {
        ViewportPtr vp = boost::dynamic_pointer_cast<Viewport>(c1);
        if ( vp ) {
            vp->SetSize(event.resize.w * 0.80, event.resize.h);
        }
        vp = boost::dynamic_pointer_cast<Viewport>(c2);
        if ( vp ) {
            vp->Set(event.resize.w * 0.8, event.resize.h*0.5, event.resize.w * 0.2, event.resize.h*0.5);
        }
        OrthoPtr ortho = boost::dynamic_pointer_cast<Ortho>(c3);
        if ( ortho ) {
            ortho->Set(event.resize.w * 0.8, 0, event.resize.w * 0.2, event.resize.h*0.5);
        }
        } break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
            processed = true;
            break;
        case SDLK_RIGHT:
            processed = true;
            break;
        case SDLK_UP:
            processed = true;
            break;
        case SDLK_DOWN:
            processed = true;
            break;
        case SDLK_HOME:
            processed = true;
            break;
        default:
            break;
        }
        break;
    case SDL_JOYBUTTONDOWN:
        switch ( event.jbutton.button ) {
        case JOY_BUTTONS::START:
            processed = true;
            break;
        default:
            break;
        }
        break;
    case SDL_JOYBUTTONUP:
        switch ( event.jbutton.button ) {
        case 0:
            break;
        default:
            break;
        }
        break;
    case SDL_JOYHATMOTION:
        switch ( event.jhat.value )
        {
        case SDL_HAT_LEFT:
            processed = true;
            break;
        case SDL_HAT_RIGHT:
            processed = true;
            break;
        case SDL_HAT_UP:
            processed = true;
            break;
        case SDL_HAT_DOWN:
            processed = true;
            break;
        }
        break;
    default:
        break;
    }
    return processed;
}

void App::InitScene( int width, int height )
{
    Renderer* renderer = dynamic_cast<Renderer*>(m_Worker.get());
    BOOST_ASSERT(renderer);
    renderer->Init();

    ////////////////////////////////////////////////////////////////////////////
    // Compose our scene
#if 0
    // Add a viewport
    EntityPtr stage(new Stage(width, height, m_Joystick ));
    // this entity renders
    renderer->AddEntity(stage);
    // listen to resize events
    m_EntityEventHandlerList.push_back( stage );
#else
    WorldPtr world( new World );

    ViewportPtr viewport2(new Viewport(width * 0.8, height*0.5, width* 0.2, height*0.5));
    viewport2->SetClearFlags(0);
    renderer->AddEntity(viewport2);

    OrthoPtr ortho(new Ortho(width * 0.8, 0, width* 0.2, height*0.5));
    renderer->AddEntity(ortho );

    // Add a viewport
    ViewportPtr viewport(new Viewport(width * 0.8, height));
    viewport->SetClearFlags(0);
    renderer->AddEntity(viewport);

    // Add the camera
    EntityPtr camera(new Camera(m_Joystick));
    // this entity handles events
    m_EntityEventHandlerList.push_back( camera );

    camera->AddEntity( world );
    // this viewport has a camera
    viewport->AddEntity(camera, 0);

    // this one does not
    viewport2->GetRenderState()->Translate( Vector( 0, 0, -10 ) );
    viewport2->AddEntity(world, 0);

    // and this is a 2D projection
    ortho->GetRenderState()->Translate( Vector( 0, 0, 0 ), Vector( 1, 1, 1 ) );
    ortho->AddEntity( world );

    // some custom event handler
    m_EventHandlerList.push_back( boost::bind( &OnHandleEvent, _1, viewport, viewport2, ortho ) );

#if 0
    LightPtr light( new Light );
    light->GetRenderState()->Translate( Vector( 0, 0, 20 ) );
    viewport->AddEntity( light, 0 );

    // setup the scene
    {
        EntityPtr e( new Cube( ) );
        e->GetRenderState()->Translate( Vector(-4.0, 3, 0), Vector(1.0f, 1.0f, 1.0f) );
        e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
        camera->AddEntity(e, 20 );
    }
    {
        EntityPtr e( new Sphere( ) );
        e->GetRenderState()->Translate( Vector(-0.0, 2, 0), Vector(1.0f, 1.0f, 1.0f) );
        e->GetRenderState()->Rotate( Vector(10.0f, 10.0f, 0.0f ) );
        camera->AddEntity(e, 20 );
    }
    {
        EntityPtr e( new Cylinder( ) );
        e->GetRenderState()->Translate( Vector(4.0, 2.5, 0), Vector(1.0f, 0.3f, 1.0f) );
        e->GetRenderState()->Rotate( Vector( 20.0f, -20.0f, 0.0f ) );
    }
#endif

#endif
}

void App::Init(int argc, char* argv[])
{
    int err = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    ASSERT( err != -1, "Failed to initialize SDL video system! SDL Error: %s\n", SDL_GetError());

//    atexit(SDL_Quit);

    int numJoysticks = SDL_NumJoysticks();
    if ( numJoysticks > 0 ) {
        m_Joystick = SDL_JoystickOpen(0);
        SDL_JoystickEventState(SDL_ENABLE);
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    int stencilSize(8);
    err = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilSize);
    ASSERT( err != -1, "Error setting stencil size to %d! SDL Error:  %s\n", stencilSize, SDL_GetError());

    int depthSize(16);
    err = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthSize);
    ASSERT( err != -1, "Error setting depth size to %d! SDL Error:  %s\n", depthSize, SDL_GetError());

    // enable multi sampling
    err = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    ASSERT( err != -1, "Error enabling multi sampling! SDL Error: %s\n", SDL_GetError());

    int numSampleBuffers(2); // test what's the max AA. test 8xMSAA
    err = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, numSampleBuffers);
    ASSERT( err != -1, "Error setting number (%d) of AA buffer! SDL Error: %s\n", numSampleBuffers, SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int vsync = 1;  // 0 = novsync
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);

    SDL_WM_SetCaption("SDL Shadowmap", NULL);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    //  SDL_WM_SetIcon( pei::SDL::SurfaceRef( pei::LogoBrush ), NULL );
    //	SDL_ShowCursor(SDL_DISABLE);
    //	SDL_EnableUNICODE(1);
}

int App::Run()
{
    int r(0);

    // somebody must attach a worker
    BOOST_ASSERT( m_Worker);

    int width(960);
    int height(540);
    SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_OPENGL|SDL_RESIZABLE);
    ASSERT( screen, "Unable to set %dx%d video! SDL Error: %s\n", width, height, SDL_GetError());

    InitScene(width, height);

    // Run our worker thread
    boost::thread worker(boost::bind(&Worker::Run, m_Worker));
    Renderer* renderer = static_cast<Renderer*>(m_Worker.get());

    bool running(true);
    SDL_Event event;
    do
    {
        int eventsPending(0);
        SDL_WaitEvent(&event);
        do
        {
            bool processed( renderer->HandleEvent( event ) );
            for ( auto entity = m_EntityEventHandlerList.begin(); entity != m_EntityEventHandlerList.end(); )
            {
                processed |= (*entity)->HandleEvent(event);
                if (processed) {
                    break;
                }
                // Remove from event handler as well if marked for deletion
                if ( (*entity)->IsFlagSet( Entity::F_DELETE ) ) {
                    entity = m_EntityEventHandlerList.erase( entity );
                    continue;
                }
                ++entity;
            }
            if (!processed)
            {
                for ( auto eventHandler : m_EventHandlerList ) {
                    processed |= eventHandler( event );
                    if (processed) {
                        break;
                    }
                }
            }
            if (!processed)
            {
                switch (event.type)
                {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_JOYBUTTONUP:
                    switch ( event.jbutton.button ) {
                    case JOY_BUTTONS::START:
                        running = false;
                        break;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                }
            }
            if (running)
            {
                eventsPending = SDL_PollEvent(&event);
            }
        } while (eventsPending > 0 && running);
    } while (running);

    // Need to clear this list before renderer destroys entities
    m_EventHandlerList.clear();

    m_Worker->Terminate();
    worker.join();

    return r;
}
