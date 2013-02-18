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

void Entity::Render( int pass ) throw(std::exception)
{
    glPushMatrix();
    glMultMatrixf( GetRenderState()->GetMatrix() );

    uint32_t flags = GetRenderState()->GetFlags();

    int glBlendSrc(0), glBlendDst(0);
    int alphaEnabled;
    glGetIntegerv(GL_ALPHA_TEST, &alphaEnabled);
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

    // render all children
    for( auto it = m_RenderList.begin(); it != m_RenderList.end(); ) {
        EntityPtr entity = *it;
        if ( entity->IsFlagSet( Entity::F_ENABLE|Entity::F_VISIBLE ) &&
            !entity->IsFlagSet( Entity::F_DELETE ) )
        {
            // don't bother rendering if we are marked for deletion
            entity->Render( pass );
        }
        ++it;
    }
    DoRender( pass );

    if (!alphaEnabled && (flags & RenderState::ALPHA_F) )
    {
        glBlendFunc( glBlendSrc, glBlendDst);
    }
    glPopMatrix();
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
