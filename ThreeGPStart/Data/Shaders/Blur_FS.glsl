/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 330

uniform sampler2D outOfFocusTexture;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(outOfFocusTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  fragColor = texture(outOfFocusTexture, texCoord);

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
          ( outOfFocusTexture
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