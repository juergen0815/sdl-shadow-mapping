/*
 * cube.h
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */

#ifndef MESH_H
#define FLAG_H


#include "err.h"
#include "entity.h"
#include "brush.h"
#include "texture.h"
#include "entitypool.h"
#include "allocator.h"

#include <vector>

class Surface : public Entity
{
public:
    enum {
        BLEND_COLOR_F = 1<<RenderState::USER_B
    };
    enum {
        TEXTURE,
        NORMAL,
        SPECULAR
    };
    enum {
        VERTEX_BUFFER = 0,
        TEXUTURE_BUFFER,
        INDEX_BUFFER,

        MAX_BUFFERS
    };

private:
    int m_Buffers[ MAX_BUFFERS ];

    enum {
        BASE_TEXTURE = 0,
        LIGHT_MAP,
        MAX_TEXTURES
    };
    std::vector< BrushPtr > m_Assets;     // all mmaped data. Don't worry about pixel storage memory here!
    std::vector< TexturePtr > m_Textures; // all textures

    typedef std::vector<Vector, Allocator<Vector>> VertexVector;

    boost::shared_ptr<MemoryPool> m_MemoryPool;

    int          m_Stride;
    VertexVector m_VertexBuffer;      // linear buffer - custom allocator - all GPU data are stored here
    std::vector<int>  m_IndexArray;   // standard array to map vertices to tris

    float       m_TimeEllapsed;
    float       m_Speed;
public:
    Surface( const std::vector< BrushPtr >& assets );

    virtual ~Surface();

protected:
    virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

    virtual void DoRender( int pass ) throw(std::exception);

    virtual void DoUpdate( float ticks ) throw(std::exception);

    void MakeSurface( int columns, int rows );
};

#endif /* MESH_H */
