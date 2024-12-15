#version 140
in vec3 worldPosition;
in vec3 worldNormal;
in vec3 worldCamera;

out vec4 fragmentColor;

uniform vec3 cameraPosition;

vec3 ambientColor = vec3(0.3, 0.3, 0.3);
vec3 diffuseColor = vec3(1.0, 1.0, 1.0);
vec3 specularColor = vec3(1.0, 1.0, 1.0);

vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 materialColor = vec3(1.0, 0.0, 0.0);

void main()
{

    //// Basic Phong
    // Ambient color computation
    vec3 ambient = ambientColor * lightColor;

    // Diffuse Color computation
    vec3 normal = normalize(worldNormal);
    vec3 light_direction = normalize(cameraPosition - worldPosition);
    float diffuse_k = max(dot(normal, light_direction), 0.0);

    vec3 diffuse = diffuseColor * diffuse_k * lightColor;

    vec3 ouput_color = ((ambient + diffuse) * materialColor);

    fragmentColor = vec4(ouput_color, 1.0);
}
