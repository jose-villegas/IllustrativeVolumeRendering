#version 400


layout(location = 0) in vec3 VerPos;
// have to use this variable!!!, or it will be very hard to debug for AMD video card
layout(location = 1) in vec3 VerClr;


out vec3 EntryPoint;
out vec4 ExitPointCoord;
out vec3 lightPos;

uniform mat4 MVP;
uniform mat4 ViewMatrix;
uniform vec3 lightPosition = vec3(5.f, -10.f, 5.f);

void main()
{
    EntryPoint = VerClr;
    lightPos = (ViewMatrix * vec4(lightPosition, 1.f)).xyz;
    gl_Position = MVP * vec4(VerPos, 1.0);
    ExitPointCoord = gl_Position;
}
