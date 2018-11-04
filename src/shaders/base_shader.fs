#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform vec3 ambientColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool norm;

void main()
{
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0); // this normal is in tangent space

    // get texture color
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = ambientColor * color;
    // diffuse
    float diff = max(dot(viewDir, normal), 0.0);
    vec3 diffuse = diff * ambientColor;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;

    if (norm)
    {
        FragColor = vec4(diffuse, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
    }
}