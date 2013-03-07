/*
 * entity.cpp
 *
 *  Created on: 2013-01-21
 *      Author: jurgens
 */

#include "entity.h"
#include "renderer.h"

#include <GL/glew.h>

#include <boost/bind.hpp>

Entity::Entity() throw ()
    : m_Flags(F_ENABLE)
    , m_OrderNum(0)
    , m_RenderState( new RenderState ) // create a default RenderState
{
}

bool Entity::CompareEntityPriorities( const EntityPtr& a, const EntityPtr& b ) {
    return a->GetOrder() < b->GetOrder();
}

RenderStatePtr Entity::GetRenderState()
{
    return m_RenderState;
}

void Entity::AddEntity( EntityPtr entity, int priority /*= 0*/  )
{
    entity->SetOrder( priority );
    m_InitList.push_back( entity );
}

bool Entity::Initialize( Renderer* renderer ) throw(std::exception)
{
    bool r(true);
    bool sortChildred( false );
    while ( m_InitList.size() ) {
        auto& entity = m_InitList.front();
        entity->Initialize( renderer );
        // should be a sorted insert...but would probably slow the container (change to set/multiset??)
        m_RenderList.push_back(entity);
        m_InitList.pop_front();
        sortChildred = true;
    }
    if ( sortChildred ) {
        m_RenderList.sort( boost::bind( &Entity::CompareEntityPriorities, this, _1, _2) );
    }
    DoInitialize( renderer );
    return r;
}

bool Entity::Update( float ticks ) throw(std::exception)
{
    if ( IsFlagSet( Entity::F_ENABLE )) {
        DoUpdate(ticks);
    }
    return false;
}

void Entity::SetupRender( int pass )
{
    // if regular mode do transform, if replay read & load projection matrix from render state
    glMatrixMode(GL_MODELVIEW); // not sure how much overhead this generates
    glPushMatrix();
    glMultMatrixf( GetRenderState()->GetMatrix() );
}

void Entity::CleanupRender( int pass )
{
    glPopMatrix();
}

void Entity::Render( int pass ) throw(std::exception)
{

    uint32_t flags = GetRenderState()->GetFlags();

    int glBlendSrc(0), glBlendDst(0);
    bool alphaEnabled = glIsEnabled(GL_ALPHA_TEST);
    if (!alphaEnabled && (flags & RenderState::ALPHA_F) )
    {
        glGetIntegerv(GL_BLEND_SRC, &glBlendSrc);
        glGetIntegerv(GL_BLEND_DST, &glBlendDst);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_ALPHA_TEST);
    }
    else if (alphaEnabled && (flags & RenderState::ALPHA_F) == 0 )
    {
        glDisable(GL_ALPHA_TEST);
    }
    int blendEnabled;
    glGetIntegerv(GL_BLEND, &blendEnabled);
    if (!blendEnabled && (flags & RenderState::BLEND_F) )
    {
        glEnable(GL_BLEND);
    }
    else if ( blendEnabled && (flags & RenderState::BLEND_F) == 0 )
    {
        glDisable(GL_BLEND);
    }

    // Do the transformation - can be over ridden for each pass.
    // Default is multiply matrix into modelview. Backs up current modelview (glPushMatrix)
    SetupRender( pass );
    // render all children
    RenderSubTree( pass );
    // restore the previous matrix. Default is a simple glPopMatrix
    CleanupRender(pass);

    if (!blendEnabled && (flags & RenderState::BLEND_F) )
    {
        // we enabled blend, disable it again
        glDisable(GL_BLEND);
    }
    if (!alphaEnabled && (flags & RenderState::ALPHA_F) )
    {
        glBlendFunc( glBlendSrc, glBlendDst);
        glDisable(GL_ALPHA_TEST);
    }
}

void Entity::RenderSubTree( int pass ) throw( std::exception )
{
    // no more transformation, etc.
    // store "projected" matrix - not the projection matrix!!!
//    glGetFloatv( GL_MODELVIEW_MATRIX, (float*)GetRenderState()->GetProjectionMatrix() );

    // render all children
    for( auto& entity : m_RenderList ) {
        if ( entity->IsFlagSet( Entity::F_ENABLE|Entity::F_VISIBLE ) &&
            !entity->IsFlagSet( Entity::F_DELETE ) )
        {
            // don't bother rendering if we are marked for deletion
            entity->Render( pass );
        }
    }
    DoRender( pass );
}


void Entity::CheckDestroy( ) throw(std::exception)
{
    for( auto it = m_RenderList.begin(); it != m_RenderList.end(); ) {
        EntityPtr entity = *it;
        // Process children first
        entity->CheckDestroy();
        // removed from list
        if ( entity->IsFlagSet( Entity::F_DELETE ) ) {
            it = m_RenderList.erase( it );
            continue;
        }
        ++it;
    }
}
