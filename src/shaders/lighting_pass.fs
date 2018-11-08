#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct PointLight {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 10;
uniform PointLight pointLights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting = vec3(0.01);; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // ambient
        vec3 ambient = Diffuse * pointLights[i].Color;
        // diffuse
        vec3 lightDir = normalize(pointLights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLights[i].Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = pointLights[i].Color * spec * Specular;
        // attenuation
        float distance = length(pointLights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + pointLights[i].Linear * distance + pointLights[i].Quadratic * distance * distance);
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += ambient + diffuse + specular;
    }
    FragColor = vec4(lighting, 1.0);
}