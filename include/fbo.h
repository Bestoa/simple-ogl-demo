#include <glad/glad.h>

class TRfbo {
    public:
        TRfbo(int w, int h);
        ~TRfbo();

        GLuint getRenderTexture() { return mRenderedTexture; }
        bool fail() { return mFail; }
        void use() {
            glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferName);
        }
        void unuse() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

    private:
        GLuint mFramebufferName;
        GLuint mRenderedTexture;
        GLuint mDepthRenderBuffer;
        bool mFail;
};
