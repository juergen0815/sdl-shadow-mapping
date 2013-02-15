#ifndef SPHERE_H
#define SPHERE_H

#include "err.h"
#include "entity.h"
#include "vector.h"

#include <vector>

class Sphere : public Entity
{
public:
    enum {
        NORMAL,
        SPECULAR
    };

private:
    int  m_VboID;
    int  m_IdxBufferID;

    typedef std::vector<Vector> VertexArray;
    typedef std::vector<Vector> ColorArray;
    typedef std::vector<unsigned int> IndexArray;

    int         m_Stride;
    VertexArray m_VertexBuffer; // linear buffer
    VertexArray m_NormalBuffer; // linear buffer
    ColorArray  m_ColorBuffer;  // color buffer overlays Vertex Array
    IndexArray  m_IndexArray;   // standard array to map vertices to tris

    float       m_Radius;
    Vector      m_Position;
    Vector      m_Scale;
    Vector      m_Rotation;
public:
    Sphere( float radius = 1.0f );

    virtual ~Sphere();
private:
    void MakeSphere( float meridians, float parallels );

protected:
    virtual bool DoInitialize( Renderer* renderer ) throw(std::exception);

    virtual void DoRender( int pass ) throw(std::exception);

    virtual void DoUpdate( float ticks ) throw(std::exception);
};

#endif /* SPHERE_H */
