#version 330 core
out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_emission1;

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

in vec3 TangentLightPos[NR_POINT_LIGHTS];

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample);

void main()
{
    vec3 normalSample   = texture(texture_normal1, fs_in.TexCoords).rgb;
    vec3 diffuseSample  = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specularSample = texture(texture_specular1, fs_in.TexCoords).rgb;
    vec3 emissionSample = texture(texture_emission1, fs_in.TexCoords).rgb;

    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 bump = normalize(normalSample * 2.0 - 1.0); // transform normal vector to range [-1,1], bump is in tangent space

    vec3 result = vec3(0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        result += CalcPointLight(pointLights[i], TangentLightPos[i], viewDir, bump, diffuseSample, normalSample, specularSample);
    }
    FragColor = vec4(result + emissionSample, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample)
{
    vec3 lightDir   = normalize(tangentLightPos - fs_in.TangentFragPos);
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
    float dist = length(light.Position - fs_in.FragPos);
    float attenuation = light.Constant / (1.0 + (light.Linear * dist) + light.Quadratic * (dist * dist));
    ambientColor  *= attenuation;
    diffuseColor  *= attenuation;
    specularColor *= attenuation * 5.0; // let the specular be less attenuated
    return (ambientColor + diffuseColor + specularColor);
}