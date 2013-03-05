/*
 * render.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef RENDER_H_
#define RENDER_H_

#include "worker.h"
#include "entity.h"

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

#include <GL/glew.h>
#ifdef __linux__
#include <GL/glx.h>
#endif
#include <SDL/SDL_syswm.h>

typedef boost::function<bool(float)> UpdateFunction;

class Renderer : public Worker
{
private:
    enum enRENDERPASS {
        RP_COLOR_BUFFER   = 1<<0,
        RP_STATIC_SHADOW  = 1<<1,
        RP_DYNAMIC_SHADOW = 1<<2,
        RP_SHADOW = RP_STATIC_SHADOW | RP_DYNAMIC_SHADOW,
        RP_REFLECTION     = 1<<3,

        RP_SYSTEM,
        RP_SYSTEM_MASK = (1<<RP_SYSTEM)-1,
    };
	bool        m_Terminate;

    EntityList  m_InitList;
	EntityList  m_RenderList;
#ifdef _WIN32
	HGLRC       m_CurrentContext;
	HDC         m_CurrentDC;
#endif
#ifdef __linux__
	GLXContext   m_CurrentContext;
#endif
	float       m_TimeBase;
	int         m_Pause;

	boost::unordered_map< long, UpdateFunction > m_Updaters;
public:
	Renderer();

	virtual ~Renderer();

	void Init();

	void AddEntity( EntityPtr entity, int priority = 0 );

	void RemoveEntity( EntityPtr entity );

	EntityPtr FindEntity( const std::string& name );

	long RegisterUpdateFunction( const UpdateFunction& func );

    void UnRegisterUpdateFunction( long key );

    virtual bool HandleEvent( const SDL_Event& event );

private:
	void InitGL();

	// No direct access
	virtual void Terminate();

	virtual void Run();

private:
	// Need this in scope of class to have access to Entity::GetPriority() method
	bool CompareEntityPriorities( const EntityPtr& a, const EntityPtr& b );

};

#endif /* RENDER_H_ */
