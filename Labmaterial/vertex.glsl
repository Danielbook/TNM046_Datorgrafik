/*//////// OLD vertex ////////
#version 330 core

uniform float time;

uniform mat4 M;

layout(location = 0) in vec3 Position;

layout(location = 1) in vec3 Color;

out vec3 interpolatedColor;

void main() {
    gl_Position = M*vec4(Position, 1.0);

    interpolatedColor = Color;
}
*/

//////// LABB 4 VERTEX ////////
#version 330 core

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 TexCoord;

uniform mat4 MV;
uniform mat4 P;


out vec3 interpolatedNormal;
out vec2 st;

void main() {
    vec3 transformedNormal = mat3(MV) * Normal;
    interpolatedNormal = normalize(transformedNormal);

    gl_Position = P*MV*vec4(Position, 1.0);
    st = TexCoord;
}

