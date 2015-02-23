#version 400
#define LIGHTING

in vec3 EntryPoint;
in vec4 ExitPointCoord;
in vec3 lightPos;

uniform mat4      NormalMatrix;
uniform sampler3D VolumeTex;
uniform sampler2D ExitPoints;
uniform sampler1D TransferFunc;
uniform float     StepSize = 0.01f;
uniform float     Threshold = 0.15f;
uniform vec2      ScreenSize;

// style transfer function uniforms
uniform sampler1D transferFunctionTexture;
uniform usampler1D indexFunctionTexture;
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

void main()
{
    vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st / ScreenSize).xyz;

    if (EntryPoint == exitPoint)//background need no raycasting
    {
        discard;
    }

    vec3 rayDirection = exitPoint - EntryPoint;
    float rayLength = length(rayDirection); // the length from front to back is calculated and used to terminate the ray
    vec3 stepVector = StepSize * rayDirection / rayLength;

    vec3 pos = EntryPoint;
    vec4 dst = vec4(0.f);
    vec3 lightDir = normalize(lightPos - EntryPoint);
    vec3 viewDir = normalize(-EntryPoint);

    while(dst.a < 1.f && rayLength > 0.f) {
		float density = texture(VolumeTex, pos).x;

		if(density > Threshold){
			// apply color
			vec4 src = texture(TransferFunc, density);
			src.rgb *= src.a;
			// apply lighting
      #ifdef LIGHTING
  			vec3 normal = (mat4(NormalMatrix) * vec4(computeGradient(pos, density), 0.f)).xyz;
  			float diffuse = lambert(normal, lightPos);
  			float specular = blinn_spec(normal, lightDir, viewDir, 16.f);
  			float ambient = 0.1f * src.rgb;
  			src.rgb = src.rgb * (diffuse + specular) + ambient;
      #endif
			// add to resuelt
			dst = (1.f - dst.a) * src + dst;
			// break;
		}
		// move further into the volume
		pos += stepVector;
		rayLength -= StepSize;
    }

    FragColor = dst;
}

// void main()
// {
//     vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st / ScreenSize).xyz;
//
//     if (EntryPoint == exitPoint)//background need no raycasting
//     {
//         discard;
//     }
//
//     vec3 dir = exitPoint - EntryPoint;
//     float len = length(dir); // the length from front to back is calculated and used to terminate the ray
//     vec3 deltaDir = normalize(dir) * StepSize;
//     float deltaDirLen = length(deltaDir);
//     vec3 voxelCoord = EntryPoint;
//     vec4 colorAcum = vec4(0.0); // The dest color
//     float alphaAcum = 0.0;                // The  dest alpha for blending
//     float intensity;
//     float lengthAcum = 0.0;
//     vec4 colorSample; // The src color
//     float alphaSample; // The src alpha
//     // backgroundColor
//     vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.0);
//
//     for (int i = 0; i < 1600; i++)
//     {
//         intensity =  texture(VolumeTex, voxelCoord).x;
//         colorSample = texture(TransferFunc, intensity);
// 		    // colorSample = vec4(intensity);
//
//         // modulate the value of colorSample.a
//         // front-to-back integration
//         if (colorSample.a > 0.0)
//         {
//             colorSample.a = 1.0 - pow(1.0 - colorSample.a, StepSize * 200.0f);
//             colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;
//             colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;
//         }
//
//         voxelCoord += deltaDir;
//         lengthAcum += deltaDirLen;
//
//         if (lengthAcum >= len)
//         {
//             colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;
//             break;  // terminate if opacity > 1 or the ray is outside the volume
//         }
//         else if (colorAcum.a > 1.0)
//         {
//             colorAcum.a = 1.0;
//             break;
//         }
//     }
//
//     FragColor = colorAcum;
//     // for test
//     // FragColor = vec4(EntryPoint, 1.0);
//     // FragColor = vec4(exitPoint, 1.0);
// }
