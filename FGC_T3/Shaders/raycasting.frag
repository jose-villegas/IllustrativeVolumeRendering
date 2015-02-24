#version 400
// #define LIGHTING
#define USE_THRESHOLD
#define AVAILABLE_STYLE_COUNT 34

in vec3 EntryPoint;
in vec4 ExitPointCoord;
in vec3 lightPos;

uniform mat4      NormalMatrix;
uniform sampler3D VolumeTex;
uniform sampler2D ExitPoints;
uniform float     StepSize = 0.01f;
uniform float     Threshold = 0.15f;
uniform vec2      ScreenSize;

// style transfer function uniforms
uniform sampler1D transferFunctionTexture;
uniform sampler1D indexFunctionTexture;
uniform sampler2DArray styleTransferTexture;

layout(location = 0) out vec4 FragColor;

vec3 computeGradient(vec3 P, float lookUp)
{
  float L = StepSize;
  float E = texture(VolumeTex, P + vec3(L,0,0));
  float N = texture(VolumeTex, P + vec3(0,L,0));
  float U = texture(VolumeTex, P + vec3(0,0,L));
  return vec3(E - lookUp, N - lookUp, U - lookUp);
}

float lambert(vec3 normal, vec3 position) {
  return max(dot(normal, lightPos), 0.f);
}

float blinn_spec(vec3 normal, vec3 lightDir, vec3 viewDir, float shininess) {
  vec3 halfDir = normalize(lightDir + viewDir);
  float specAngle = max(dot(halfDir, normal), 0.f);
  return pow(specAngle, shininess);
}

vec2 litsphere(vec3 normal, vec3 position) {
  return vec2(normal.x, -normal.y) * 0.5f + 0.5f;
}

vec2 litsphere(vec3 normal, vec3 position, float len) {
  return vec2(normal.x, -normal.y) * (1 - len) + len;
}

vec2 matcap(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);

  float m = 2.0 * sqrt(
    pow(reflected.x, 2.0) +
    pow(reflected.y, 2.0) +
    pow(reflected.z + 1.0, 2.0)
  );

  return reflected.xy / m + 0.5;
}

void main()
{
  vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st / ScreenSize).xyz;

  if (EntryPoint == exitPoint) discard;//background need no raycasting

  vec3 rayDirection = exitPoint - EntryPoint;
  float rayLength = length(rayDirection); // the length from front to back is calculated and used to terminate the ray
  vec3 stepVector = StepSize * rayDirection / rayLength;

  vec3 pos = EntryPoint;
  vec4 dst = vec4(0.f);
  vec3 normal = vec3(1.f);
  vec4 baseColor = vec4(1.f);
  vec4 src = vec4(1.f);

  while(dst.a < 1.f && rayLength > 0.f) {
    float density = texture(VolumeTex, pos).x;

    #ifdef USE_THRESHOLD
      if(density > Threshold) {
    #endif

    float opacity = texture(transferFunctionTexture, density).g;
    float index = texture(transferFunctionTexture, density).r;
    int styleIndex = int(texture(indexFunctionTexture, index).x * AVAILABLE_STYLE_COUNT);
    vec3 previousNormal = normal;

    // apply litsphere
    normal = (mat4(NormalMatrix) * vec4(computeGradient(pos, density), 0.f)).xyz;
    baseColor = texture(styleTransferTexture, vec3(litsphere(normal, pos).xy, styleIndex));

    // calculate curvate approximation
    float magnitudes = length(normal) * length(previousNormal);
    float curvature = acos(dot(normal, previousNormal) / magnitudes) * StepSize;

    // apply contour
    float thickness = 1.f;
    float Tkv = thickness * curvature;
    float cond = sqrt(Tkv * (2.f - Tkv));
    float nDotV = abs(dot(normal, normalize(-pos)));

    if(nDotV <= cond) {
      float litDelta = 1.f - min(1.f, (cond - nDotV) / cond);
      float adjustedLength = min(1.f, length(normal) / litDelta);
      // weird trick to use matcap shader making contours show off
      baseColor = texture(styleTransferTexture, vec3(matcap(ExitPointCoord.xyz, normal).xy, styleIndex));
      // baseColor = texture(styleTransferTexture, vec3(litsphere(normal, pos, adjustedLength).xy, styleIndex));
      // baseColor.rgb = vec3(0.0, 0.0, 0.0); // testing with pure color
    }

    // src value
    src = vec4(baseColor.rgb, opacity);

    // add lighting
    #ifdef LIGHTING
      float diffuse = lambert(normal, lightPos);
      float ambient = 0.1f * src.rgb; // fake ambient light
      src.rgb = (src.rgb * diffuse) + ambient;
    #endif

    // add to result
    src.rgb *= src.a;
    dst = (1.f - dst.a) * src + dst;

    #ifdef USE_THRESHOLD
      }
    #endif

    // move further into the volume
    pos += stepVector;
    rayLength -= StepSize;
  }

  FragColor = dst;
}
