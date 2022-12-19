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