//type fragment
#version 450

layout(location = 0) out vec4 o_Color;
in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_Pos;

uniform vec3 u_LightPosition;
uniform vec3 u_CameraPosition;

void main(){
    vec3 color = v_Color.rgb;
    // ambient
    vec3 ambient = 0.05 * color;

    // diffuse
    vec3 lightDir = normalize(u_LightPosition - v_Pos);
    vec3 normal = normalize(v_Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // specular
    vec3 viewDir = normalize(u_CameraPosition - v_Pos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    bool blinn = true;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    o_Color = vec4(ambient + diffuse + specular, 1.0);

}
