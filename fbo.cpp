#include <iostream>
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "fbo.h"

using namespace std;
using namespace glm;

#define MULTISAMPLE 4

TRfbo::TRfbo (int w, int h)
{
    mFail = true;
    glGenFramebuffers(1, &mFramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferName);

    glGenTextures(1, &mRenderedTexture);
#if !MULTISAMPLE
    glBindTexture(GL_TEXTURE_2D, mRenderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderedTexture, 0);
#else
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mRenderedTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTISAMPLE, GL_RGB, w, h, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mRenderedTexture, 0);
#endif

    glGenRenderbuffers(1, &mDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderBuffer);
#if !MULTISAMPLE
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
#else
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLE, GL_DEPTH_COMPONENT16, w, h);
#endif
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderBuffer);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        mFail = false;
}

TRfbo::~TRfbo()
{
    glDeleteFramebuffers(1, &mFramebufferName);
    glDeleteTextures(1, &mRenderedTexture);
    glDeleteRenderbuffers(1, &mDepthRenderBuffer);
}
