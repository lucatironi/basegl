#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    mat4 projectionView = projection * view;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));

    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    Normal = normalMatrix * aNormal;
    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * FragPos;

    gl_Position = projectionView * worldPos;
}