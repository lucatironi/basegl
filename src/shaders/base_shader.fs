#version 330 core
out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_emission1;

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

uniform vec3 viewPos;

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLight;

uniform bool dirLightFlag;
uniform bool bumpFlag;

in vec3 TangentLightPos[NR_POINT_LIGHTS];

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample, vec3 emissionSample);
vec3 CalcDirLight(DirLight light, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample, vec3 emissionSample);

void main()
{
    // vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 norm = normalize(fs_in.Normal);

    vec3 diffuseSample  = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 normalSample   = texture(texture_normal1, fs_in.TexCoords).rgb;
    vec3 specularSample = texture(texture_specular1, fs_in.TexCoords).rgb;
    vec3 emissionSample = texture(texture_emission1, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1], bump is in tangent space
    vec3 bump = normalize(normalSample * 2.0 - 1.0);
    float diff = max(dot(viewDir, bump), 0.0);

    vec3 result = vec3(0.01);
    if (dirLightFlag)
    {
        result = CalcDirLight(dirLight, viewDir, bump, diffuseSample, normalSample, specularSample, emissionSample);
    }
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], TangentLightPos[i], viewDir, bump, diffuseSample, normalSample, specularSample, emissionSample);
    }
    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample, vec3 emissionSample)
{
    vec3 lightDir   = normalize(tangentLightPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(viewDir, bump), 0.0);
    // specular shading
    float spec = pow(max(dot(bump, halfwayDir), 0.0), 16.0);

    // vec3 lightDir = normalize(light.Position - fs_in.FragPos);
    // vec3 reflectDir = reflect(-lightDir, bump);
    // // diffuse shading
    // float diff = max(dot(bump, lightDir), 0.0);
    // // specular shading
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);

    // combine results
    vec3 ambientColor  = light.Color * diffuseSample;
    vec3 diffuseColor  = light.Color * diff * diffuseSample;
    vec3 specularColor = light.Color * spec * specularSample;
    // attenuation
    float dist = length(light.Position - fs_in.FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * dist + light.Quadratic * (dist * dist));
    ambientColor  *= attenuation;
    diffuseColor  *= attenuation;
    specularColor *= attenuation;
    return (ambientColor + diffuseColor + specularColor + emissionSample);
}

// calculates the fragment color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 viewDir, vec3 bump, vec3 diffuseSample, vec3 normalSample, vec3 specularSample, vec3 emissionSample)
{
    vec3 lightDir   = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(viewDir, bump), 0.0);
    // specular shading
    float spec = pow(max(dot(bump, halfwayDir), 0.0), 16.0);

    // vec3 lightDir = normalize(-light.Direction);
    // vec3 reflectDir = reflect(-lightDir, bump);
    // // diffuse shading
    // float diff = max(dot(bump, lightDir), 0.0);
    // // specular shading
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);

    // combine results
    vec3 ambientColor  = light.Ambient  * diffuseSample;
    vec3 diffuseColor  = light.Diffuse  * diff * diffuseSample;
    vec3 specularColor = light.Specular * spec * specularSample;
    return (ambientColor + diffuseColor + specularColor + emissionSample);
}