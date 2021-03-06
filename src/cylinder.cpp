#include "cylinder.h"

#include <GL/glew.h>

#include <cmath>

#include <boost/filesystem.hpp>

const int _columns = 32;
const int _rows    = 2;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Cylinder::Cylinder( float length /* = 1.0f */ )
    : m_Buffers( { -1 } )
    , m_Stride(1) // needed if/when we pack color + vertex into one array
    , m_VertexBuffer( _columns*_rows*m_Stride )    // use the same memory pool for vertex and texture coords
    , m_ColorBuffer( _columns*_rows*m_Stride )
    , m_Radius(1.0f)
{
}

Cylinder::~Cylinder()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    glDeleteBuffers( MAX_BUFFERS, (GLuint*)m_Buffers);
}

void Cylinder::SetColors( const Vector& colorFrom, const Vector& colorTo )
{
    m_ColorFrom = colorFrom;
    m_ColorTo   = colorTo;
}

void Cylinder::MakeCylinder( float columns, float rows )
{
    const float RAD360 = M_PI*2; // 2*PI in RAD

    int lastColumn = columns - 1;
    int lastRow = rows;
    ++rows; // one extra row to top off the poly

    // Add two extra vertices at center bottom and top
    m_VertexBuffer.resize( columns*rows*m_Stride + 2 );
    m_NormalBuffer.resize( columns*rows*m_Stride + 2 );
    m_ColorBuffer.resize( columns*rows*m_Stride + 2 );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( columns * rows * 3 * 2 + columns*3*2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    const float height = 6;
    auto vit = m_VertexBuffer.begin();
    auto nit = m_NormalBuffer.begin();
    auto cit = m_ColorBuffer.begin();
    int looper(0);

    // need one extra ring to close the gap (overlaps 0)
    float segmentSize  = RAD360/columns;
    for( float y = 0; y < rows; ++y ){  // must <= because 2 "rows" are actually 3 vertex rings
        std::vector< Vector > segments;
        float vpy = y / lastRow * height - height/2;
        for( float x = 0; x < columns; ++x ) { //0-2PI
            float phi = x * segmentSize;
            // vertex
            auto& vertex = *vit; ++vit;
            // Cylinder
            vertex[ Vector::X ] = std::cos(phi) * m_Radius; //std::cos(theta) * std::sin(phi);
            vertex[ Vector::Y ] = vpy; // std::sin(theta) * std::cos(phi);
            vertex[ Vector::Z ] = std::sin(phi) * m_Radius; // std::cos(phi);

            // Add normal vectors - at vertex direction from center (at y pos)
            auto& normal = *nit; ++nit;
            normal = Vector( vertex ).Sub( {  0, vpy, 0  } ).Normalize();

            // vertex color
            auto& color = *cit; ++cit;
            color = { 1.0f - y/rows, 1.0f, y/rows, 1.0f };

            // skip last column/row - already indexed
            if ( y < lastRow ) {
                // vertices don't need to be set just yet. We just index them here

                // top tri
                int
                idx = int((int(x + 0) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx;  // 0x0
                idx = int((int(x + 1) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx;  // 1x0
                idx = int((int(x + 0) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

                // bottom tri
                idx = int((int(x + 1) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx; // 0x0
                idx = int((int(x + 1) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
                idx = int((int(x + 0) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
            }
        }
    }

    // we share vertices with first and last ring -> problem with normals. This point away from center, only center is correct
    auto& vb = *vit; ++vit;
    vb = { 0,-height/2, 0 };   // bottom - center
    auto& nb = *nit; ++nit;
    nb = { 0, -1, 0 };         // point down
    auto& cb = *cit; ++cit;
    cb = { 1.0f, 1.0f, 0.0f, 1.0f };
    int bottomIdx = columns * rows * m_Stride;
    // close top and bottom
    for( int x = 0; x < columns; ++x ) { //0-2PI
        // bottom
        int
        idx = (x + 0) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 0x0 - readability!
        idx = (x + 1) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 1x0
        idx = bottomIdx;            m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row
    }
    auto& vt = *vit; ++vit;
    vt = { 0, height/2, 0 };   // top - center
    auto& nt = *nit; ++nit;
    nt = { 0, +1, 0 };       // point up
    auto& ct = *cit; ++cit;
    ct = { 0.0f, 1.0f, 1.0f, 1.0f };
    int topIdx = bottomIdx+1;
    for( int x = 0; x < columns; ++x ) { //0-2PI
        int
        idx = (x + 1) % lastColumn + columns*lastRow; m_IndexArray[ looper++ ] = idx;  // 1x0
        idx = (x + 0) % lastColumn + columns*lastRow; m_IndexArray[ looper++ ] = idx;  // 0x0 - readability!
        idx = topIdx;               m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row
    }
}

bool Cylinder::DoInitialize( Renderer* renderer ) throw(std::exception)
{
    bool hasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );

    // we might just want to create this in DoInitialize - and throw away the data we don't need locally
    MakeCylinder( _columns, _rows );

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

void Cylinder::DoUpdate( float ticks ) throw(std::exception)
{
}

void Cylinder::DoRender( int pass ) throw(std::exception)
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

