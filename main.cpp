#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "objs.h"
#include "fbo.h"

using namespace std;
using namespace glm;

#define WIDTH (1280)
#define HEIGHT (720)

#define RENDER_TO_TEXTURE 1

TRObj *create_floor()
{
    VertexData v[6] =
    {
        {
            { -1.0f, -1.0f, -1.0f, 1.0f },
            { 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
        {
            { -1.0f, -1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
        {
            { 1.0f, -1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
        {
            { 1.0f, -1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
        {
            { 1.0f, -1.0f, -1.0f, 1.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
        {
            { -1.0f, -1.0f, -1.0f, 1.0f },
            { 0.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f },
            { 255, 255, 255, 255 },
        },
    };
    TRObj *obj = new TRObj(v, 6);
    obj->loadTexture("res/floor_diffuse.tga", TEXTURE_DIFFUSE);
    obj->loadDummyTexture(TEXTURE_SPECULAR);
    return obj;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Demo", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create window" << endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    TRObj *floor = create_floor();
    TRObj *obj = new TRObj("1.obj");
    if (!obj || obj->fail())
    {
        cout << "Create OBJ failed." << endl;
        return -1;
    }
    obj->loadTexture("1.tga", TEXTURE_DIFFUSE);
    obj->loadDummyTexture(TEXTURE_SPECULAR);

    Shader shader("shaders/obj.vert", "shaders/obj.frag");
    shader.use();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    mat4 model_mat(1.0f);

    mat4 view_mat = lookAt(
                vec3(0,1,2), // Camera is at (0,1,2), in World Space
                vec3(0,0,0), // and looks at the origin
                vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                );

    mat4 projection_mat = perspective(radians(75.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Light position in camera space
    vec3 light_pos = vec3(view_mat * vec4(1.0f));
    shader.setVec3("light.position", light_pos);
    shader.setVec4("light.color", vec4(1.0f));
    shader.setFloat("light.ambient_strength", 0.1f);
    shader.setInt("light.shininess", 32);

    mat4 floor_model_mat4 = mat4(1.0f);

#if RENDER_TO_TEXTURE
    // Create FBO
    TRfbo *fbo = new TRfbo(WIDTH, HEIGHT);
    fbo->unuse();

    // The fullscreen quad's FBO
    GLuint quad_vao;
    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);

    Shader quad_shader("shaders/quad.vert", "shaders/quad.frag");
#endif

    int frame = 0;
    while (!glfwWindowShouldClose(window))
    {

#if 1
        // Model is rotating
        model_mat = rotate(model_mat, radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
#endif
#if 0
        // Eye is rotating
        view_mat = lookAt(
                vec3(sin(radians(1.0f) * (frame % 360)) * 2, 1, cos(radians(1.0f) * (frame % 360)) * 2),
                vec3(0, 0, 0),
                vec3(0, 1, 0)
                );
        light_pos = vec3(view_mat * vec4(1.0f));
        shader.setVec3("light.position", light_pos);
#endif
#if 0
        // Light is rotating
        light_pos = vec3(view_mat * vec4(sin(radians(1.0f) * (frame % 360)), 1, cos(radians(1.0f) * (frame % 360)), 1.0f));
        shader.setVec3("light.position", light_pos);
#endif

#if RENDER_TO_TEXTURE
        fbo->use();
        shader.use();
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WIDTH, HEIGHT);
#endif

        shader.setMat4("model_mat", model_mat);
        shader.setMat4("view_mat", view_mat);
        shader.setMat4("projection_mat", projection_mat);

        mat4 normal_mat = transpose(inverse(view_mat * model_mat));
        shader.setMat4("normal_mat", normal_mat);

        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        obj->drawMesh(shader);

        shader.setMat4("model_mat", floor_model_mat4);
        floor->drawMesh(shader);

#if RENDER_TO_TEXTURE
        fbo->unuse();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        quad_shader.use();
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f);
        glClear( GL_COLOR_BUFFER_BIT );
        glBindVertexArray(quad_vao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo->getRenderTexture());

        quad_shader.setInt("renderedTexture", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glViewport(0, 0, WIDTH/5, HEIGHT/5);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame++;
    }

#if RENDER_TO_TEXTURE
    delete fbo;
#endif
    delete obj;
    delete floor;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
