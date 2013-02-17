/*
 * cube.cpp
 *
 *  Created on: Dec 4, 2011
 *      Author: Jurgen
 */


#include "surface.h"
#include "cube.h"
#include "brush.h"
#include "brushloader.h"

#include <cmath>

#include <boost/filesystem.hpp>

const int sCcolumns = 120;
const int sRows    = 120;

struct PoolDeleter
{
    void operator()(void const *p) const
    {
        // called when shared_ptr goes out of scope - that is last container releases link to pool
        EntityPool::DestroyPool( (MemoryPool*)p );
    }
};

Surface::Surface( const std::vector< BrushPtr >& assets )
    : m_Buffers( { -1 } )
    , m_Assets( assets )
    , m_MemoryPool( EntityPool::CreatePool<Vector>( 0 ), PoolDeleter() )
    , m_Stride(2)// store two vectors per vertex
    , m_VertexBuffer( m_MemoryPool )    // use the same memory pool for vertex and texture coords
    , m_TimeEllapsed(0)
    , m_Speed(1.0)
{
    m_Textures.resize( MAX_TEXTURES );
    m_Textures = { TexturePtr() };

    // we might just want to create this in DoInitialize - and throw away the data we don't need locally

    // allocate memory buffers for vertex and texture coord
    m_VertexBuffer.resize( sCcolumns*sRows*m_Stride );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( (sRows-1) * (sCcolumns-1) * 3 * 2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    int looper(0);
    // width x height is always a quad, not a rect
    const float width_2 = 15.0f; // 4.4f - for columns = 45
    const float depth_2 = 15.0f;

    // x/z - y  up. Surface lays flat

    // generate vertex array
    const float xstep = (2*width_2)/sCcolumns; // mesh sub divider - 0.2f
    const float zstep = (2*depth_2)/sRows; // mesh sub divider - 0.2f
    const float amp  = 0.85f; // "height" of wave
    const float numWaves = 16.0f; // num of sin loops (or waves)
    auto vit = m_VertexBuffer.begin();

    // I think we need an additional row/column to finish this mesh ??
    for ( float z = 0; z < sRows; ++z )
    {
        for ( float x = 0; x < sCcolumns; ++x )
        {
            Vector& vertex = *vit; ++vit;
            vertex[ Vector::X ] = x * xstep - width_2; // -4.4 ... +4.4
            // maybe I should shift this for each row, huh, norm x to "length" of column (0.0 - 1.0)
            vertex[ Vector::Y ] = std::sin( (x/sCcolumns) * numWaves ) * amp; // make z a big "wavy" -- fix this. Must be multiple if sin(360)
            vertex[ Vector::Z ] = z * zstep - depth_2; // -4.4 ... +4.4

            // calc texture positions
            Vector& texCoord = *vit; ++vit;
            texCoord[ Vector::U ] = x/(sCcolumns-1);
            texCoord[ Vector::V ] = z/(sRows-1);

            // this needs work: we use a row * col vertex and texture array
            // to extract triangles, the index array needs to be calculated appropriately
            //        0  1  2...n
            //        +--+--+...
            //        |\ |\ |
            //        | \| \|
            //        +--+--+
            // n*y +  0' 1' 2'...(n+1)*y

            // e.g. t[0] = { 0,1,1'} { 1',0',1 } ...

            // skip last column/row - already indexed
            if ( x < (sCcolumns-1) && z < (sRows-1) ) {
                // vertices don't need to be set just yet. We just index them here

                // top tri
                int
                idx = int(x + 0 + sCcolumns*z);     m_IndexArray[ looper++ ] = idx;  // 0x0
                idx = int(x + 1 + sCcolumns*z);     m_IndexArray[ looper++ ] = idx;  // 1x0
                idx = int(x + 0 + sCcolumns*(z+1)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

                // bottom tri
                idx = int(x + 1 + sCcolumns*z);     m_IndexArray[ looper++ ] = idx; // 0x0
                idx = int(x + 0 + sCcolumns*(z+1)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
                idx = int(x + 1 + sCcolumns*(z+1)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
                idx = 0;
            }
        }
    }
}

Surface::~Surface()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    glDeleteBuffers( MAX_BUFFERS, (GLuint*)m_Buffers );
}

bool Surface::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    bool hasMultiTexture  = glewGetExtension("GL_ARB_multitexture");
    ASSERT( hasMultiTexture, "No multitexture support!" );

    int numTextureUnits;
    glGetIntegerv( GL_MAX_TEXTURE_UNITS, &numTextureUnits );
    ASSERT( MAX_TEXTURES <= numTextureUnits, "Not enough texture units available." );

    std::size_t texi = 0;
    for ( auto& asset : m_Assets ) {
        if ( texi > m_Textures.size() ) break;

        m_Textures[texi] = TexturePtr( new Texture );
        auto& texture = m_Textures[texi]; ++texi;
        texture->Load( *asset.get() );
    }
    GetRenderState()->ClearFlag( BLEND_COLOR_F );

    bool hasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );

    glGenBuffers( MAX_BUFFERS, (GLuint*)m_Buffers );

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[ VERTEX_BUFFER ] );
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector)*m_VertexBuffer.size(), 0, GL_STATIC_DRAW);
    std::size_t offset(0);

    // ***! INTERLEAVED!! copy vertices starting from 0 offest - holds both, vertex and texture array
    float *vertices = (float*)&m_VertexBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_VertexBuffer.size(), vertices);

    // Index Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[ INDEX_BUFFER ]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_IndexArray.size(), &m_IndexArray[0], GL_STATIC_DRAW);

    return true;
}

void Surface::DoRender( int pass ) throw(std::exception)
{

    int vertexArrayEnabled;
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    int texCoordArrayEnabled;
    glGetIntegerv( GL_TEXTURE_COORD_ARRAY, &texCoordArrayEnabled );
    if ( m_Textures.size() && !texCoordArrayEnabled ) {
        if (!texCoordArrayEnabled) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    // no need for textures in shadow pass - for now. -> transparent shadows will need it
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[ VERTEX_BUFFER ]);
    glVertexPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), 0);
    // Base texture. No special treatment. Just draw it
    if ( m_Textures[BASE_TEXTURE] ) {
        glClientActiveTexture(GL_TEXTURE0);
        // only use u/v coords, skip t/s - stride is from n[0] + offset = n[1],
        glTexCoordPointer( 2, GL_FLOAT, m_Stride*sizeof(Vector), (void*)sizeof(Vector) ); // first Vector is vertex

        glActiveTexture(GL_TEXTURE0);
        m_Textures[BASE_TEXTURE]->Enable();
    }
    // Lightmap. Simple RGB blend it into previous texture
    if ( m_Textures[LIGHT_MAP] ) {
        glClientActiveTexture(GL_TEXTURE1);
        // only use u/v coords, skip t/s - stride is from n[0] + offset = n[1]
        glTexCoordPointer( 2, GL_FLOAT, m_Stride*sizeof(Vector), (void*)sizeof(Vector) );

        glActiveTexture(GL_TEXTURE1);
        m_Textures[LIGHT_MAP]->Enable();

        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_BLEND);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    }

    // use index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[ INDEX_BUFFER ]);
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, (void*)0 );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }

    if ( m_Textures[BASE_TEXTURE] ) {
        m_Textures[BASE_TEXTURE]->Disable();
    }
    if ( m_Textures[LIGHT_MAP] ) {
        m_Textures[LIGHT_MAP]->Disable();
    }
    if (!texCoordArrayEnabled) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

}

void Surface::DoUpdate( float ticks ) throw(std::exception)
{
    m_TimeEllapsed += ticks;
    if ( m_TimeEllapsed*m_Speed > 16.67 )
    {
        m_TimeEllapsed = 0;
        // size must be > 2
        auto first = m_VertexBuffer[0]; // backup first vertex
        for ( auto vit = m_VertexBuffer.begin(); vit != m_VertexBuffer.end();  ) {
            auto& vertex = *vit; vit += m_Stride;
            vertex[ Vector::Y ] = (*vit)[ Vector::Y ];
        }
        auto& last = m_VertexBuffer[ sCcolumns*sRows*m_Stride-m_Stride];
        last[ Vector::Y ] = first[ Vector::Y ];

        // I should probably use split buffers to not copy tex coords again and again
        glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[ VERTEX_BUFFER ] );
        // ***! INTERLEAVED!! copy vertices starting from 0 offest - holds both, vertex and texture array
        float *vertices = (float*)&m_VertexBuffer[0];
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector)*m_VertexBuffer.size(), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
