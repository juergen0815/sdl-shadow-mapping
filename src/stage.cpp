/*
 * stage.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */


#include "stage.h"

Stage::Stage( int width, int height, SDL_Joystick* joystick )
    : m_Overlay( new Ortho(0,0,width,height))
    , m_MainStage( new Viewport )
    , m_ShadowMapStage( new Ortho )
    , m_LightingStage( new Viewport )
    , m_Camera( new Camera(joystick) )
    , m_World( new World )
    , m_ShadowMap( FrameBuffer::F_ENABLE_DEPTH_BUFFER_F ) // depth buffer only!
{
    m_Camera->AddEntity( m_World );

    // camera is attached to main stage
    m_MainStage->AddEntity( m_Camera );
    m_MainStage->SetClearColor( Vector(1.0, 0.0, 0.0, 1.0 ) );
    m_MainStage->SetClearFlags(0);

    m_LightingStage->AddEntity( m_World );
    m_LightingStage->SetClearColor( Vector(1.0, 1.0, 0.0, 1.0 ) );
    m_LightingStage->SetClearFlags(0);

    m_ShadowMapStage->SetClearColor( Vector(0.0, 1.0, 0.0, 1.0 ) );
    m_ShadowMapStage->SetClearFlags(0);

    OnResize( width, height );
}

Stage::~Stage()
{

}

bool Stage::Initialize( Renderer* renderer ) throw(std::exception)
{
    bool r = Entity::Initialize( renderer );

    r &= m_MainStage->Initialize( renderer );
    r &= m_ShadowMapStage->Initialize( renderer );
    r &= m_LightingStage->Initialize( renderer );

    return r;
}

bool Stage::DoInitialize( Renderer* renderer ) throw( std::exception )
{
    glEnable(GL_LIGHTING);

    // create offscreen shadowmap
    int width(1024);
    int height(1024);
    bool r = m_ShadowMap.Allocate( width, height );
    ASSERT( r, "Cannot allocate shadow map!");

    return true;
}

void Stage::Render( int pass ) throw(std::exception)
{
    int lighting(false);
    glGetIntegerv(GL_LIGHTING, &lighting );

    glMatrixMode(GL_MODELVIEW); // not sure how much overhead this generates
    glPushMatrix();
    glMultMatrixf( GetRenderState()->GetMatrix() );

//    glClearColor( 0, 0, 0, 0 ); // background color
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use a lookup table so we can actually reorder these if needed, or add new, or w/e
    int renderPasses[] = {
//            PASS_SHADOW_MAP,
            PASS_LIGHTING,
//            PASS_SHADOW_TEST
    };
    int renderPass(0);
    int numPasses( sizeof(renderPasses)/sizeof(int));
    do {

        int p = renderPasses[renderPass];
        switch ( p )
        {
        case PASS_SHADOW_MAP: {
            // render from light pos into depth map
            const World::LightList& lights = m_World->GetLights();
            // no shadow without light
            if ( lights.size() > 0 ) {
                // do not use lighting for shadow map
                glDisable(GL_LIGHTING);

                // enable shadow texture
                m_ShadowMap.Enable();

                // set viewport to texture size
                glViewport( 0, 0, (GLsizei)m_ShadowMap.GetWidth(), (GLsizei)m_ShadowMap.GetHeight());

                // set perspective viewing frustum - copy from main stage
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf( m_MainStage->GetRenderState()->GetProjectionMatrix() );

                // switch to modelview matrix in order to set scene
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                for ( const auto& light : lights ) {
                    glPushMatrix();
                    // 1) project light
                    glMultMatrixf( (float*)light->GetRenderState()->GetMatrix() );
                    // 2) render world from light position into offscreen buffer - need to set the frustum for the world
                    //    Only depth component! no texturing, etc
                    m_World->Render( p );
                    glPopMatrix();
                }
                m_ShadowMap.Disable();
            }

            // TODO: Render texture into shadow stage

            // switch lighting back on
            if ( lighting ) glEnable(GL_LIGHTING);
            }break;
        case PASS_LIGHTING:
        default:
            // default lighting pass - render whole scene
            if ( !lighting ) glEnable(GL_LIGHTING);
            m_MainStage->Render( p );
            // Render lighting "window"
            m_LightingStage->Render( p );
            // render shadow map into 2D window
            m_ShadowMapStage->Render( PASS_LIGHTING );
            break;
        case PASS_SHADOW_TEST:
            // render shadows from shadow map into scene
            if ( !lighting ) glEnable(GL_LIGHTING);
            // m_MainStage->Render( p );
            break;
        }

    } while (++renderPass < numPasses );


    // render overlay
//    m_Overlay->Render( PASS_LIGHTING );

    if ( !lighting ) glDisable(GL_LIGHTING);

    glPopMatrix();
}

void Stage::DoRender( int pass ) throw( std::exception )
{
    // not really much to do here. This would be used if the stage would want to render something (like, hub, or info or w/e)
}

void Stage::OnResize( int width, int height )
{
    m_Overlay->Set(0,0,width, height);

    // MainStage covers left portion to 80%
    float w = float(width) * 0.8f;
    float h = float(height);
    m_MainStage->SetSize( w, h );
    // Right top half is shadow map
    float x = w; w = width - w;
    float y = 0;
    h *= 0.5;
    m_ShadowMapStage->Set( x, y, w, h );
    // right bottom half is non-shadowed scene
    y += h;
    m_LightingStage->Set( x, y, w, h );

}

bool Stage::HandleEvent( const SDL_Event& event )
{
    bool eventHandled(false);
    switch (event.type) {
    case SDL_VIDEORESIZE:
        OnResize(event.resize.w, event.resize.h);
        break;
    default:
        break;
    }
    if ( !eventHandled ) {
        eventHandled = m_Camera->HandleEvent( event );
    }
    return eventHandled;
}
