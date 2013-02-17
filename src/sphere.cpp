#include "sphere.h"

#include <GL/glew.h>

#include <cmath>

#include <boost/filesystem.hpp>

const int _columns = 32;
const int _rows    = 12;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923f
#endif

Sphere::Sphere( float radius /* = 1.0f */ )
    : m_Buffers( { -1 } )
    , m_Stride(1) // needed if/when we pack color + vertex into one array
    , m_VertexBuffer( _columns*_rows*m_Stride )    // use the same memory pool for vertex and texture coords
    , m_ColorBuffer( _columns*_rows*m_Stride )
    , m_Radius(radius)
    , m_Color( { 0.75, 0.75, 0.75 } )
{
}

Sphere::~Sphere()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    glDeleteBuffers(MAX_BUFFERS, (GLuint*)m_Buffers);
}

void Sphere::SetColor( const Vector& color )
{
    m_Color = color;
}

void Sphere::MakeSphere( float columns, float rows )
{
    const float RAD180 = M_PI; // PI in RAD
    const float RAD360 = M_PI*2; // 2*PI in RAD

    ++rows;
    int lastColumn = columns - 1;
    int lastRow = rows - 1;

    // Add two extra vertices at center bottom and top
    m_VertexBuffer.resize( columns*rows*m_Stride );
    m_NormalBuffer.resize( columns*rows*m_Stride );
    m_ColorBuffer.resize( columns*rows*m_Stride );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( columns * rows * 3 * 2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    auto vit = m_VertexBuffer.begin();
    auto nit = m_NormalBuffer.begin();
    auto cit = m_ColorBuffer.begin();
    int looper(0);

    // from http://www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/learn.htm

    // need one extra ring to close the gap (overlaps 0)
    float segmentAngle = RAD180/lastRow;
    float segmentSize  = RAD360/columns;
    for( float y = 0; y < rows; ++y ){  //0-PI
        std::vector< Vector > segments;
        float theta = y * segmentAngle;
        for( float x = 0; x < columns; ++x ) { //0-2PI
            float phi = x * segmentSize;
            // vertex
            auto& vertex = *vit; ++vit;
            vertex[ Vector::X ] = m_Radius * std::sin(phi) * std::cos(theta);
            vertex[ Vector::Y ] = m_Radius * std::sin(phi) * std::sin(theta);
            vertex[ Vector::Z ] = m_Radius * std::cos(phi);

            // Add normal vectors - at vertex direction from center (-{0,0,0})
            auto& normal = *nit; ++nit;
            normal = vertex.Normalized();

            // vertex color
            auto& color = *cit; ++cit;
            color = m_Color;

            // vertices don't need to be set just yet. We just index them here

            // this needs work: we use a row * col vertex and texture array
            // to extract triangles, the index array needs to be calculated appropriately
            //        0  1  2...n
            //        +--+--+...
            //        |\ |\ |
            //        | \| \|
            //        +--+--+
            // n*y +  0' 1' 2'...(n+1)*y

            // e.g. t[0] = { 0,1,1'} { 1',0',1 } ...
            // top tri
            int
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 0x0
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 1x0
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

            // bottom tri
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 0x0
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
        }
    }
}

bool Sphere::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    bool hasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );

    // we might just want to create this in DoInitialize - and throw away the data we don't need locally
    MakeSphere( _columns, _rows );

    glGenBuffers( MAX_BUFFERS, (GLuint*)m_Buffers);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[ VERTEX_BUFFER ]);
    int bufSize = sizeof(Vector)*m_VertexBuffer.size()
                + sizeof(Vector)*m_NormalBuffer.size()
                + sizeof(Vector)*m_ColorBuffer.size();
    glBufferData(GL_ARRAY_BUFFER, bufSize, 0, GL_STATIC_DRAW);
    std::size_t offset(0);
    // Vertex buffer
    float *vertices = (float*)&m_VertexBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_VertexBuffer.size(), vertices);
    // Normal buffer
    offset += sizeof(Vector)*m_VertexBuffer.size();
    float *normals = (float*)&m_NormalBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_NormalBuffer.size(), normals);
    // Color buffer
    offset += sizeof(Vector)*m_NormalBuffer.size();
    float *colors = (float*)&m_ColorBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_ColorBuffer.size(), colors);

    // Index Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[ INDEX_BUFFER ]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_IndexArray.size(), &m_IndexArray[0], GL_STATIC_DRAW);

    // TODO: We can delete local storage here

    return true;
}

void Sphere::DoUpdate( float ticks ) throw(std::exception)
{
}

void Sphere::DoRender( int pass ) throw(std::exception)
{
    int vertexArrayEnabled;
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    int normalArrayEnabled;
    glGetIntegerv( GL_NORMAL_ARRAY, &normalArrayEnabled );
    if ( !normalArrayEnabled )  {
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    int colorArrayEnabled;
    glGetIntegerv( GL_COLOR_ARRAY, &colorArrayEnabled );
    if ( !colorArrayEnabled) {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[ VERTEX_BUFFER ]);
    // before draw, specify vertex and index arrays with their offsets
    std::size_t offset(0);
    glVertexPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    offset += sizeof(Vector)*m_VertexBuffer.size();
    glNormalPointer(GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    offset += sizeof(Vector)*m_NormalBuffer.size();
    glColorPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    // use index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[ INDEX_BUFFER ]);
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, (void*)0 );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }
    if ( !normalArrayEnabled )  {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if ( !colorArrayEnabled) {
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

