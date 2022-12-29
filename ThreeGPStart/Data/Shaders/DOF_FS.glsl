///*
//  (C) 2019 David Lettier
//  lettier.com
//*/
//
//#version 330
//
//uniform sampler2D positionTexture;
//uniform sampler2D noiseTexture;
//uniform sampler2D focusTexture;
//uniform sampler2D outOfFocusTexture;
//
//uniform vec2 mouseFocusPoint;
//uniform vec2 nearFar;
//uniform vec2 enabled;
//
//out vec4 fragColor;
//out vec4 fragColor1;
//
//void main() {
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
//  fragColor  = mix(focusColor, outOfFocusColor, blur);
//  fragColor1 = vec4(blur);
//}

/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 330

uniform sampler2D sampler_tex;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(sampler_tex, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  fragColor = texture(sampler_tex, texCoord);

  int size = int(parameters.x);
  if (size <= 0) { return; }

  float separation = parameters.y;
        separation = max(separation, 1);

  fragColor.rgb = vec3(0);

  float count = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      fragColor.rgb +=
        texture
          ( sampler_tex
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          ).rgb;

      count += 1.0;
    }
  }

  fragColor.rgb /= count;
}

//#version 330
//
//uniform sampler2D sampler_tex;
//
//out vec4 color;
//
//uniform float focal_distance;
//uniform float focal_depth;
//
//void main(void)
//{
//    // s will be used to scale our texture coordinates before
//    // looking up data in our SAT image.
//    vec2 s = 1.0 / textureSize(sampler_tex, 0);
//    // C is the center of the filter
//    vec2 C = gl_FragCoord.xy;
//
//    // First, retrieve the value of the SAT at the center
//    // of the filter. The last channel of this value stores
//    // the view-space depth of the pixel.
//    vec4 v = texelFetch(sampler_tex, ivec2(gl_FragCoord.xy), 0).rgba;
//
//    // M will be the radius of our filter kernel
//    float m;
//
//    // For this application, we clear our depth image to zero
//    // before rendering to it, so if it's still zero we haven't
//    // rendered to the image here. Thus, we set our radius to 
//    // 0.5 (i.e., a diameter of 1.0) and move on.
//    if (v.w == 0.0)
//    {
//        m = 0.5;
//    }
//    else
//    {
//        // Calculate a circle of confusion
//        m = abs(v.w - focal_distance);
//
//        // Simple smoothstep scale and bias. Minimum radius is
//        // 0.5 (diameter 1.0), maximum is 8.0. Box filter kernels
//        // greater than about 16 pixels don't look good at all.
//        m = 0.5 + smoothstep(0.0, focal_depth, m) * 7.5;
//    }
//
//    // Calculate the positions of the four corners of our
//    // area to sample from.
//    vec2 P0 = vec2(C * 1.0) + vec2(-m, -m);
//    vec2 P1 = vec2(C * 1.0) + vec2(-m, m);
//    vec2 P2 = vec2(C * 1.0) + vec2(m, -m);
//    vec2 P3 = vec2(C * 1.0) + vec2(m, m);
//
//    // Scale our coordinates.
//    P0 *= s;
//    P1 *= s;
//    P2 *= s;
//    P3 *= s;
//
//    // Fetch the values of the SAT at the four corners
//    vec3 a = textureLod(sampler_tex, P0, 0).rgb;
//    vec3 b = textureLod(sampler_tex, P1, 0).rgb;
//    vec3 c = textureLod(sampler_tex, P2, 0).rgb;
//    vec3 d = textureLod(sampler_tex, P3, 0).rgb;
//
//    // Calculate the sum of all pixels inside the kernel.
//    vec3 f = a - b - c + d;
//
//    // Scale radius -> diameter.
//    m *= 2;
//
//    // Divide through by area
//    f /= float(m * m);
//
//    // Outut final color
//    color = vec4(f, 1.0);
//}