#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;

#define NR_POINT_LIGHTS 10
struct PointLight {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};

struct DirLight {
    vec3 Direction;

    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLight;

out vec3 TangentLightPos[NR_POINT_LIGHTS];

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.TangentLightPos = TBN * dirLight.Direction;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        TangentLightPos[i] = TBN * pointLights[i].Position;
    }

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}