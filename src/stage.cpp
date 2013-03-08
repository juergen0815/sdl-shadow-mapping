/*
 * stage.cpp
 *
 *  Created on: 2013-03-04
 *      Author: jurgens
 */


#include "stage.h"

class DrawRectangle : public Entity
{
    TexturePtr          m_Texture;
    std::vector<Vector> m_VertexArray; // rectangle vertex array
    bool                m_RefreshVertexArray; // don't really need to store this

    class RenderState : public ::RenderState
    {
    public:
        float m_X, m_Y, m_Z;
        float m_Width, m_Height;

        RenderState()
            : m_X(0), m_Y(0), m_Z(0)
            , m_Width(0), m_Height(0)
        {}

    } *m_RenderStateProxy;


public:
    DrawRectangle()
        : m_VertexArray( 2*6 )
        , m_RefreshVertexArray(false)
        , m_RenderStateProxy( new RenderState )
    {
        m_RenderState = RenderStatePtr( m_RenderStateProxy );
    }

    virtual ~DrawRectangle()
    {

    }

    void SetTexture( TexturePtr tex )
    {
        m_Texture = tex;
    }

    void SetPosition( int x, int y, int z = 0 )
    {
        m_RenderStateProxy->m_X = (float)x;
        m_RenderStateProxy->m_Y = (float)y;
        m_RenderStateProxy->m_Z = (float)z;
        m_RefreshVertexArray = true;
    }

    void SetSize( int w, int h )
    {
        m_RenderStateProxy->m_Width  = (float)w;
        m_RenderStateProxy->m_Height = (float)h;
        m_RefreshVertexArray = true;
    }

protected:
    bool DoInitialize( Renderer* renderer ) throw( std::exception )
    {
        UpdateVertexArray();
        return true;
    }

    void UpdateVertexArray()
    {
        float x0 = m_RenderStateProxy->m_X;
        float y0 = m_RenderStateProxy->m_Y;
        float z0 = m_RenderStateProxy->m_Z;

        float x1 = m_RenderStateProxy->m_X + m_RenderStateProxy->m_Width;
        float y1 = m_RenderStateProxy->m_Y + m_RenderStateProxy->m_Height;

        // pair of vertex & texture
        m_VertexArray = { { x0, y0, z0 }, { 0,0,0 },
                          { x1, y0, z0 }, { 0,1,0},
                          { x1, y1, z0 }, { 0,1,1 },

                          { x1, y1, z0 }, { 0,1,1 },
                          { x0, y1, z0 }, { 0,1,0 },
                          { x1, y0, z0 }, { 0,1,0},
                        };
        m_RefreshVertexArray = false;

    }

    void DoRender( int pass ) throw( std::exception )
    {
        if ( m_RefreshVertexArray ) {
            UpdateVertexArray();
        }

        int stride = 2; // interleaved vec/tex

        int vertexArrayEnabled;
        glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
        if (!vertexArrayEnabled) {
            glEnableClientState(GL_VERTEX_ARRAY);
        }

        // no need for textures in shadow pass - for now. -> transparent shadows will need it
        glVertexPointer(4, GL_FLOAT, stride*sizeof(Vector), &m_VertexArray[0] );
        // Base texture. No special treatment. Just draw it
        if ( m_Texture ) {
            int texCoordArrayEnabled;
            glGetIntegerv( GL_TEXTURE_COORD_ARRAY, &texCoordArrayEnabled );
            if ( !texCoordArrayEnabled ) {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }

            // interleave, 4 component vector, skip 2
            glTexCoordPointer( 4, GL_FLOAT, stride*sizeof(Vector), (void*)&m_VertexArray[1] );

            m_Texture->Enable();
            glDrawArrays( GL_TRIANGLES, 0, 6 );
            m_Texture->Disable();

            if ( !texCoordArrayEnabled ) {
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        } else {
            glDrawArrays( GL_TRIANGLES, 0, 6 );
        }

        if (!vertexArrayEnabled) {
            glDisableClientState(GL_VERTEX_ARRAY);
        }

    }
};

Stage::Stage( int width, int height, SDL_Joystick* joystick )
    : m_Overlay( new Ortho(0,0,width,height))
    , m_MainStage( new Viewport )
    , m_ShadowMapStage( new Ortho )
    , m_LightingStage( new Viewport )
    , m_Camera( new Camera(joystick) )
    , m_ShadowProjection( new Viewport )
    , m_World( new World )
    , m_ShadowMap( FrameBuffer::F_ENABLE_DEPTH_BUFFER_F | FrameBuffer::F_ENABLE_COLOR_BUFFER_F ) // depth buffer only!
{
    // camera is attached to main stage
    m_MainStage->AddEntity( m_Camera );
    m_Camera->AddEntity( m_World );
    m_MainStage->SetClearColor( Vector(1.0, 0.0, 0.0, 1.0 ) );
    m_MainStage->SetClearFlags(0);

    m_LightingStage->AddEntity( m_World );
    m_LightingStage->SetClearColor( Vector(0.0, 1.0, 0.0, 1.0 ) );
    m_LightingStage->SetClearFlags(0);

    m_ShadowProjection->AddEntity( m_World );
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
    r &= m_ShadowProjection->Initialize(renderer);

    // this is just a preview of the shadow map
    r &= m_ShadowMapStage->Initialize( renderer );

    // this is just a plain render
    r &= m_LightingStage->Initialize( renderer );

    return r;
}

bool Stage::DoInitialize( Renderer* renderer ) throw( std::exception )
{
    glEnable(GL_LIGHTING);

    // Default viewport (used for camera)
    m_MainStage->Reset( 45.0f, 1.0f, 100.0f );
    // create offscreen shadowmap
    int width(1024);
    int height(1024);
    bool r = m_ShadowMap.Allocate( width, height );
    ASSERT( r, "Cannot allocate shadow map!");

    // "virtual viewport"
    m_ShadowProjection->SetSize( width, height );
    m_ShadowProjection->Reset( 45.0f, 1.0f, 8.0f );

    m_ShadowRect = DrawRectanglePtr( new DrawRectangle );
    m_ShadowRect->SetTexture( m_ShadowMap.GetTexture() );
    m_ShadowRect->SetSize( m_ShadowMapStage->GetWidth(), m_ShadowMapStage->GetHeight() );
    m_ShadowMapStage->AddEntity( m_ShadowRect );

    return true;
}

void Stage::Render( int pass ) throw(std::exception)
{
    SetupRender( pass );

    int lighting(false);
    glGetIntegerv(GL_LIGHTING, &lighting );

//    glClearColor( 0, 0, 0, 0 ); // background color
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use a lookup table so we can actually reorder these if needed, or add new, or w/e
    int renderPasses[] = {
//            PASS_SHADOW_MAP, // grey scale pass without lighting
            PASS_LIGHTING,  // default pass with lighting
//            PASS_SHADOW_TEST
    };
    int renderPass(0);
    int numPasses( sizeof(renderPasses)/sizeof(int));
    do {

        int p = renderPasses[renderPass];
        unsigned int passMask = 1<<p;
        switch ( p )
        {
        case PASS_SHADOW_MAP: {
            // render from light pos into depth map
            const World::LightList& lights = m_World->GetLights();
            // no shadow without light
            if ( lights.size() > 0 ) {
                // do not use lighting for shadow map
                glDisable(GL_LIGHTING);

                // enable shadow texture - render into here
                m_ShadowMap.Enable();

                // set viewport to texture size
                glViewport( 0, 0, (GLsizei)m_ShadowMap.GetWidth(), (GLsizei)m_ShadowMap.GetHeight());

                // set perspective viewing frustum - copy from main stage
                glMatrixMode(GL_PROJECTION);
                // shouldn't I re-calc the frustum???
                glLoadMatrixf( m_MainStage->GetRenderState()->GetProjectionMatrix() );

                // switch to modelview matrix in order to set scene
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();

                // clear texture (depth only)
                glClear(GL_DEPTH_BUFFER_BIT);

                // TODO: Tune GL to remove all unnecessary renderings
                //       - no back faces
                //       - no textures

                // re-render the tree from each light position. Depth only, additive into same texture
                for ( const auto& light : lights ) {
                    // disable light
                    unsigned int flags = light->ClearFlags( ~0 );
                    // move viewport to light position
                    m_ShadowProjection->GetRenderState()->SetMatrix( light->GetRenderState()->GetMatrix() );
                    // render from light
                    m_ShadowProjection->Render( passMask );
                    // re-enable it
                    light->SetFlags( flags );
                    // 1) project light
//                    glLoadMatrixf( (float*)light->GetRenderState()->GetMatrix() );
                    // 2) render world from light position into offscreen buffer - need to set the frustum for the world
                    //    Only depth component! no texturing, etc
//                    m_World->Render( passMask ); // passMask );
                }
                glPopMatrix();
                m_ShadowMap.Disable();

                // render shadow map into 2D window - this only draws depth components...must overlay with some depth test grey scale or something (otherwise must copy depth component into grey texture)
                // Keeps a texture instance - no need to update
                m_ShadowMapStage->Render( PASS_LIGHTING_F );

                // switch lighting back on
                if ( lighting ) glEnable(GL_LIGHTING);
            }

            // TODO: Render texture into shadow stage

            }break;
        case PASS_LIGHTING:
        default:
            // default lighting pass - render whole scene
            if ( !lighting ) {
                glEnable(GL_LIGHTING);
            }
            m_MainStage->Render( passMask );
            // Render lighting "window"
            m_LightingStage->Render( passMask );
            break;
        case PASS_SHADOW_TEST:
            // render shadows from shadow map into scene
            if ( !lighting ) {
                glEnable(GL_LIGHTING);
            }
            // m_MainStage->Render( passMaskp );
            break;
        }

    } while (++renderPass < numPasses );


    // render overlay
    m_Overlay->Render( PASS_LIGHTING_F );

    if ( !lighting ) glDisable(GL_LIGHTING);

    CleanupRender( pass );
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
    // this is ortho
    m_ShadowMapStage->Set( x, y, w, h );
    // the 2D texture to show the shadow texture
    m_ShadowRect->SetSize( w, h );

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
