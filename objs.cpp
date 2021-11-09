#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "objs.h"
#include "shader.h"

using namespace std;
using namespace glm;

const GLubyte sprite_color[3][4] = {
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 },
};

void TRObj::__create_mesh__(VertexData *vertices)
{
    cout << "Create TRObj..." << endl;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * mNumVertices, vertices, GL_STATIC_DRAW);

    if (glGetError() != GL_NO_ERROR)
        goto error;

    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), GET_STRUCT_OFFSET(VertexData, position) );
    glEnableVertexAttribArray( vPosition );

    glVertexAttribPointer( vTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), GET_STRUCT_OFFSET(VertexData, texcoord) );
    glEnableVertexAttribArray( vTexcoord );

    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), GET_STRUCT_OFFSET(VertexData, normal) );
    glEnableVertexAttribArray( vNormal );

    glVertexAttribPointer( vColor , 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), GET_STRUCT_OFFSET(VertexData, color) );
    glEnableVertexAttribArray( vColor );

    for (int i = 0; i < TEXTURE_TYPENUM; i++)
    {
        mTextureID[i] = UINT_MAX;
    }

    mFail = false;
    cout << "OK." << endl;
    return;

error:
    cout << "Failed." << endl;
    glDeleteBuffers(1, &mVBO);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mVAO);
}

TRObj::TRObj(VertexData *vertices, size_t size)
{
    mFail = true;

    mNumVertices = size;
    __create_mesh__(vertices);
    mName = "anon_obj";
}


TRObj::TRObj(const char *obj_name)
{
    mFail = true;

    VertexData *vertices = nullptr;
    if (!__load_obj__(obj_name, vertices))
    {
        cout << "Load OBJ " << obj_name << " error." << endl;
        return;
    }

    __create_mesh__(vertices);
    // Do not forget to free it.
    delete [] vertices;
    mName = obj_name;
}

TRObj::~TRObj()
{
    cout << "Desotry TRObj " << mName << endl;
    if (mFail)
        return;
    glDeleteBuffers(1, &mVBO);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mVAO);

    for (int i = 0; i < TEXTURE_TYPENUM; i++)
    {
        if (mTextureID[i] != UINT_MAX)
            glDeleteTextures(1, &mTextureID[i]);
    }
}

bool TRObj::loadTexture(const char *name, GLuint type)
{
    bool ret = true;

    cout << "Load texture image " << name << "..." << endl;

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(name, &width, &height, &nrChannels, 0);

    if (data == nullptr)
    {
        cout << "Failed." << endl;
        return false;
    }

    ret = loadTexture(data, width, height, nrChannels, type);

    stbi_image_free(data);
    cout << (ret ? "OK." : "Failed.") << endl;

    return ret;
}

bool TRObj::loadTexture(GLubyte *data, GLuint w, GLuint h, GLuint c, GLuint type)
{
    if (mTextureID[type] != UINT_MAX)
    {
        cout << "Already load texture, type = " << type << endl;
        return false;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    // Do not bind it to texture 0
    glActiveTexture(GL_TEXTURE31);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (c == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else if (c == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else
    {
        cout << "Only support RGB888/RGBA8888 texture." << endl;
        goto error;
    }

    if (glGetError() != GL_NO_ERROR)
        goto error;
    glActiveTexture(GL_TEXTURE0);

    mTextureID[type] = texture;
    return true;

error:
    glDeleteTextures(1, &texture);
    return false;
}

void TRObj::loadDummyTexture(GLuint type)
{
    // 1x1 black texutre
    GLubyte black[] = { 0, 0, 0 };
    GLubyte normal[] = { 127, 127, 255 };

    if (type != TEXTURE_NORMAL)
        loadTexture(black, 1, 1, 3, type);
    else
        loadTexture(normal, 1, 1, 3, type);
}

bool TRObj::drawMesh(Shader &shader)
{
    glBindVertexArray(mVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureID[TEXTURE_DIFFUSE]);
    shader.setInt("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTextureID[TEXTURE_SPECULAR]);
    shader.setInt("material.specular", 1);

    glDrawArrays(GL_TRIANGLES, 0, mNumVertices);
    return true;
}

// Copied from http://www.opengl-tutorial.org/
// Modified by us.
bool TRObj::__load_obj__(
        const char * path,
        VertexData * &vertices
        )
{
    cout << "Loading OBJ file " << path << "..." << endl;

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    int faces = 0;

    FILE * file = fopen(path, "r");
    if (file == NULL)
    {
        cout << "Impossible to open the obj file ! Are you in the right path ?" << endl;
        return false;
    }

    while(true)
    {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader

        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            uv.y = uv.y;
            temp_uvs.push_back(uv);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%u/%u/%u %u/%u/%u %u/%u/%u\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                cout << "File can't be read by our simple parser :-( Try exporting with other options" << endl;
                fclose(file);
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
            faces++;
        }else{
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }

    }
    printf("Faces: %d\n", faces);

    mNumVertices = vertexIndices.size();
    vertices = new VertexData[mNumVertices];
    if (vertices == nullptr)
    {
        cout << "Alloc vertices failed." << endl;
        fclose(file);
        return false;
    }

    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){

        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
        glm::vec3 normal = temp_normals[ normalIndex-1 ];

        vertices[i] = {
            { vertex.x, vertex.y, vertex.z, 1.0f },
            { uv.x, uv.y },
            { normal.x, normal.y, normal.z, 1.0f },
            { sprite_color[i % 3][0], sprite_color[i % 3][1], sprite_color[i % 3][2], sprite_color[i % 3][3] }
        };

    }
    fclose(file);
    cout << "OK." << endl;
    return true;
}
