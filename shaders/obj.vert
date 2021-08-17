#version 330 core

layout ( location = 0 ) in vec4 vPosition;
layout ( location = 1 ) in vec2 vTexcoord;
layout ( location = 2 ) in vec4 vNormal;
layout ( location = 3 ) in vec4 vColor;

out vec2 texcoord;
out vec4 color;
out vec3 normal;
out vec3 frag_pos;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;
uniform mat4 normal_mat;

void
main()
{
    texcoord = vTexcoord;
    color = vColor;
    normal = (normal_mat * vNormal).xyz;
    vec4 frag_pos4f = view_mat * model_mat * vPosition;
    frag_pos = frag_pos4f.xyz;
    gl_Position = projection_mat * frag_pos4f;
}
