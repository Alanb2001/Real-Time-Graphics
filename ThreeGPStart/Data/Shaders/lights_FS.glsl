#version 330

struct Lights
{  
    vec3 light_colour;
    vec3 light_position;
};

#define MAXLIGHTS 3

uniform sampler2D sampler_tex;
uniform samplerCube shadowMap;
uniform vec3 camera_position;
uniform Lights light[MAXLIGHTS];
uniform float farPlane;

in vec3 varying_position; 
in vec3 varying_normal;
in vec2 varying_coord;

out vec4 fragment_colour;

vec3 PointLight(Lights light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float light_range = 3000;
    float material_shininess = 20;

    vec3 material_diffuse_colour = texture(sampler_tex, varying_coord).rgb;
    vec3 fragment_position = fragPos;
    vec3 fragment_normal = normalize(normal);

    vec3 material_ambient_colour = material_diffuse_colour;
    vec3 material_specular_colour = material_diffuse_colour;

    vec3 light_direction = normalize(light.light_position - fragPos);
    vec3 camera_direction = normalize(viewDir - fragPos);

    float light_distance = distance(light.light_position, fragPos);
    float attenuation = 1.0 - smoothstep(0, light_range, light_distance);

    float light_amount = max(0, dot(light_direction, fragment_normal));
    vec3 diffuse_colour = material_diffuse_colour * light_amount;

    vec3 rV = reflect(-light_direction, fragment_normal);
    float LR = max(0, dot(camera_direction, rV));
    vec3 specular_colour = material_specular_colour * pow(LR, material_shininess);

    //float shadow = 0.0;
    //vec3 fragToLight = varying_position - light.light_position;
    //float currentDepth = length(fragToLight);
    //float bias = max(0.5f * (1.0f - dot(varying_normal, light_direction)), 0.0005f);
    //
    //int sampleRadius = 2;
    //float offset = 0.02f;
    //for(int z = -sampleRadius; z <= sampleRadius; z++)
	//{
    //    for(int y = -sampleRadius; y <= sampleRadius; y++)
    //    {
    //        for(int x = -sampleRadius; x <= sampleRadius; x++)
	//		{
    //            float closestDepth = texture(shadowMap, fragToLight + vec3(x, y, z) * offset).r;
    //            closestDepth *= farPlane;
    //            if(currentDepth > closestDepth + bias)
    //            {
    //                shadow += 1.0f;
    //            }  
	//		}
    //    }
	//}
    //
    //shadow /= pow((sampleRadius * 2 + 1), 3);

    vec3 final_colour = light.light_colour * (diffuse_colour + specular_colour) * attenuation;

    return (final_colour);
}

void main(void)
{
    vec3 norm = normalize(varying_normal);
    vec3 viewDir = normalize(camera_position - varying_position);

    vec3 result = vec3(0);

    for(int i = 0; i < MAXLIGHTS; i++)
    {
       result += PointLight(light[i], norm, varying_position, viewDir);
    }

    fragment_colour = vec4(result, 1.0);
}