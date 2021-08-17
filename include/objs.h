#ifndef __OBJS__
#define __OBJS__

#include <iostream>
#include <glad/glad.h>
#include "shader.h"

enum {
    TEXTURE_DIFFUSE,
    TEXTURE_SPECULAR,
    TEXTURE_NORMAL,
    TEXTURE_TYPENUM,
};

// Please refer to the obj.vert
enum {
    vPosition,
    vTexcoord,
    vNormal,
    vColor,
};

struct VertexData {
    GLfloat position[4];
    GLfloat texcoord[2];
    GLfloat normal[4];
    GLubyte color[4];
};

#define GET_STRUCT_OFFSET(s, f) ((void *)&((s *)0)->f)

class TRObj
{
    public:
        TRObj(VertexData *vertices, size_t size);
        TRObj(const char *obj_name);
        ~TRObj();
        GLuint getVAO() { return mVAO; }
        GLuint getVerticesNum() { return mNumVertices; }
        GLuint getTextureID(GLuint type) { return mTextureID[type]; }
        bool fail() { return mFail; }
        bool loadTexture(const char *texture_name, GLuint type);
        bool loadTexture(GLubyte *texture, GLuint width, GLuint height, GLuint channel, GLuint type);
        void loadDummyTexture(GLuint type);
        bool drawMesh(Shader &shader);

    private:
        GLuint mVAO;
        GLuint mVBO;
        GLuint mTextureID[TEXTURE_TYPENUM];
        GLuint mNumVertices;

        bool mFail;

        bool __load_obj__(const char *name, VertexData * &vertices);
        void __create_mesh__(VertexData *vertices);
};
#endif
