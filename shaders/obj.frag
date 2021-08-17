#version 330 core
#extension GL_ARB_shading_language_420pack : require

in vec2 texcoord;
in vec4 color;
in vec3 normal;
in vec3 frag_pos;

out vec4 fColor;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
};

uniform Material material;

struct Light
{
    // light position in camera space.
    vec3 position;
    vec4 color;
    float ambient_strength;
    int shininess;
};

uniform Light light;

void main()
{
    vec4 base_color = texture(material.diffuse, texcoord);
    vec4 ambient = base_color * light.ambient_strength;
    fColor = ambient;

    vec3 n_normal = normalize(normal);
    vec3 n_light_dir = normalize(light.position - frag_pos);

    vec4 diffuse = max(dot(n_normal, n_light_dir), 0.0) * light.color * base_color;
    fColor += diffuse;

    vec3 n_view_dir = normalize(-frag_pos);
    vec3 n_reflect_dir = reflect(-n_light_dir, n_normal);

    float spec = pow(max(dot(n_view_dir, n_reflect_dir), 0.0), light.shininess);
    vec4 specular = texture(material.specular, texcoord) * spec * light.color;
    fColor += specular;
}
