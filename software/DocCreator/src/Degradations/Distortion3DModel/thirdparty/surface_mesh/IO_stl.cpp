//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011-2013 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


//== INCLUDES =================================================================

#include <surface_mesh/IO.h>

#include <cstdio>
#include <cfloat>
#include <map>


//== NAMESPACES ===============================================================


namespace surface_mesh {


//== IMPLEMENTATION ===========================================================


// helper function
template <typename T> void read(FILE* in, T& t)
{
    size_t n_items(0);
    n_items = fread((char*)&t, 1, sizeof(t), in);
    assert(n_items > 0);
}


//-----------------------------------------------------------------------------


// helper class for STL reader
class CmpVec
{
public:

    CmpVec(float _eps=FLT_MIN) : eps_(_eps) {}

    bool operator()(const Vec3f& v0, const Vec3f& v1) const
    {
        if (fabs(v0[0] - v1[0]) <= eps_)
        {
            if (fabs(v0[1] - v1[1]) <= eps_)
            {
                return (v0[2] < v1[2] - eps_);
            }
            else return (v0[1] < v1[1] - eps_);
        }
        else return (v0[0] < v1[0] - eps_);
    }

private:
    float eps_;
};


//-----------------------------------------------------------------------------


bool read_stl(Surface_mesh& mesh, const std::string& filename)
{
    char                            line[100], *c;
    unsigned int                    i, nT;
    Vec3f                           p;
    Surface_mesh::Vertex               v;
    std::vector<Surface_mesh::Vertex>  vertices(3);
    size_t n_items(0);

    CmpVec comp(FLT_MIN);
    std::map<Vec3f, Surface_mesh::Vertex, CmpVec>            vMap(comp);
    std::map<Vec3f, Surface_mesh::Vertex, CmpVec>::iterator  vMapIt;


    // clear mesh
    mesh.clear();


    // open file (in ASCII mode)
    FILE* in = fopen(filename.c_str(), "r");
    if (!in) return false;


    // ASCII or binary STL?
    c = fgets(line, 6, in);
    assert(c != NULL);
    const bool binary = ((strncmp(line, "SOLID", 5) != 0) &&
                         (strncmp(line, "solid", 5) != 0));


    // parse binary STL
    if (binary)
    {
        // re-open file in binary mode
        fclose(in);
        in = fopen(filename.c_str(), "rb");
        if (!in) return false;

        // skip dummy header
        n_items = fread(line, 1, 80, in);
        assert(n_items > 0);

        // read number of triangles
        read(in, nT);

        // read triangles
        while (nT)
        {
            // skip triangle normal
            n_items = fread(line, 1, 12, in);
            assert(n_items > 0);
            // triangle's vertices
            for (i=0; i<3; ++i)
            {
                read(in, p);

                // has vector been referenced before?
                if ((vMapIt=vMap.find(p)) == vMap.end())
                {
                    // No : add vertex and remember idx/vector mapping
                    v = mesh.add_vertex((Point)p);
                    vertices[i] = v;
                    vMap[p] = v;
                }
                else
                {
                    // Yes : get index from map
                    vertices[i] = vMapIt->second;
                }
            }

            // Add face only if it is not degenerated
            if ((vertices[0] != vertices[1]) &&
                (vertices[0] != vertices[2]) &&
                (vertices[1] != vertices[2]))
                mesh.add_face(vertices);

            n_items = fread(line, 1, 2, in);
            assert(n_items > 0);
            --nT;
        }
    }


    // parse ASCII STL
    else
    {
        // parse line by line
        while (in && !feof(in) && fgets(line, 100, in))
        {
            // skip white-space
            for (c=line; isspace(*c) && *c!='\0'; ++c) {};

            // face begins
            if ((strncmp(c, "outer", 5) == 0) ||
                (strncmp(c, "OUTER", 5) == 0))
            {
                // read three vertices
                for (i=0; i<3; ++i)
                {
                    // read line
                    c = fgets(line, 100, in);
                    assert(c != NULL);

                    // skip white-space
                    for (c=line; isspace(*c) && *c!='\0'; ++c) {};

                    // read x, y, z
                    sscanf(c+6, "%f %f %f", &p[0], &p[1], &p[2]);

                    // has vector been referenced before?
                    if ((vMapIt=vMap.find(p)) == vMap.end())
                    {
                        // No : add vertex and remember idx/vector mapping
                        v = mesh.add_vertex((Point)p);
                        vertices[i] = v;
                        vMap[p] = v;
                    }
                    else
                    {
                        // Yes : get index from map
                        vertices[i] = vMapIt->second;
                    }
                }

                // Add face only if it is not degenerated
                if ((vertices[0] != vertices[1]) &&
                    (vertices[0] != vertices[2]) &&
                    (vertices[1] != vertices[2]))
                    mesh.add_face(vertices);
            }
        }
    }


    fclose(in);
    return true;
}


//=============================================================================
} // namespace surface_mesh
//=============================================================================
