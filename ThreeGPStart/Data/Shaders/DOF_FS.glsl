/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 330

//uniform sampler2D positionTexture;
//uniform sampler2D focusTexture;
//uniform sampler2D outOfFocusTexture;
//
//uniform vec2 mouseFocusPoint;
//uniform vec2 nearFar;
//uniform vec2 enabled;
//
//layout (location = 0) out vec4 fragColor;
//layout (location = 1) out vec4 fragColor1;
//
//void main() 
//{
//  float minDistance =  8.0;
//  float maxDistance = 12.0;
//
//  float far  = nearFar.y;
//
//  vec2 texSize  = textureSize(focusTexture, 0).xy;
//  vec2 texCoord = gl_FragCoord.xy / texSize;
//
//  vec4 focusColor = texture(focusTexture, texCoord);
//
//  fragColor = focusColor;
//
//  if (enabled.x != 1) { return; }
//
//  vec4 position = texture(positionTexture, texCoord);
//
//  if (position.a <= 0) { fragColor1 = vec4(1.0); return; }
//
//  vec4 outOfFocusColor = texture(outOfFocusTexture, texCoord);
//  vec4 focusPoint      = texture(positionTexture,   mouseFocusPoint);
//
//  float blur =
//    smoothstep
//      ( minDistance
//      , maxDistance
//      , length(position - focusPoint)
//      );
//
//  fragColor  = mix(focusColor, outOfFocusColor, blur); //vec4(1.0);
//  fragColor1 = vec4(blur); //vec4(1.0);
//}

out vec3 FragColor;
in vec2 varying_coord;

uniform sampler2D shadedPass; //Image to be processed
uniform sampler2D linearDistance;
uniform vec2 pixelSize; //The size of a pixel: vec2(1.0/width, 1.0/height)

uniform float focus;
uniform float aperture;
uniform float focalLength;
uniform int iterations;
uniform int apertureBlades;
uniform float bokehSqueeze;
uniform float bokehSqueezeFalloff;
uniform float aspectRatio;

const float MAX_BLUR_SIZE = 20.0;

float getBlurSize(float depth) 
{
    return abs(
        (focalLength * (focus - depth)) /
        (depth * (focus - focalLength))
    ) * (focalLength / aperture) * 10000.0;
}

float rand(vec2 co) 
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 rand2(vec2 co) 
{
    return vec2(rand(co), rand(co * 20.0)) * 2.0 - 1.0;
}

void main() 
{
    float centerDepth = texture(linearDistance, varying_coord).r;
    float centerBlur = getBlurSize(centerDepth);
    vec3 color = texture(shadedPass, varying_coord).rgb;
    for (int i = 0; i < iterations; i++) 
    {
        vec2 offset = rand2(varying_coord + i) * centerBlur;
        vec2 uv = varying_coord + offset * pixelSize;
        color += texture(shadedPass, uv).rgb;
    }
    color /= float(iterations);
    FragColor = color;
    // FragColor = vec3(centerBlur);
}