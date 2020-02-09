#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmission;

struct PointLight {
    vec3 Position;
    vec3 Color;

    float Constant;
    float Linear;
    float Quadratic;
};

const int NR_POINT_LIGHTS = 14;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, float specularSample);

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos         = texture(gPosition, TexCoords).rgb;
    vec3 normalSample    = texture(gNormal, TexCoords).rgb;
    vec3 diffuseSample   = texture(gAlbedoSpec, TexCoords).rgb;
    float specularSample = texture(gAlbedoSpec, TexCoords).a;
    vec3 emissionSample  = texture(gEmission, TexCoords).rgb;

    vec3 tangentLightPos = vec3(0.0);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 bump = normalize(normalSample * 2.0 - 1.0); // transform normal vector to range [-1,1], bump is in tangent space

    vec3 result = vec3(0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        result += CalcPointLight(pointLights[i], FragPos, viewDir, bump, diffuseSample, normalSample, specularSample);
    }
    FragColor = vec4(result + emissionSample, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, float specularSample)
{
    vec3 lightDir   = normalize(light.Position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(viewDir, bump), 0.0);
    // specular shading
    float spec = pow(max(dot(bump, halfwayDir), 0.0), 8.0);

    // combine results
    vec3 ambientColor  = light.Color * diffuseSample;
    vec3 diffuseColor  = light.Color * diff * diffuseSample;
    vec3 specularColor = light.Color * spec * specularSample;

    // attenuation
    float dist = length(light.Position - fragPos);
    float attenuation = light.Constant / (1.0 + (light.Linear * dist) + light.Quadratic * (dist * dist));
    ambientColor  *= attenuation;
    diffuseColor  *= attenuation;
    specularColor *= attenuation * 5.0; // let the specular be less attenuated
    return (ambientColor + diffuseColor + specularColor);
}