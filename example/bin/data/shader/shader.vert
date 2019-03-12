#version 420
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 texcoord;

uniform float lds;
out vec4 vPos;
out float vDepth;

void main() {

    vec3 p = position.xyz;
    vPos = modelViewMatrix * vec4(p, 1.);
    vDepth = - vPos.z * lds;
    gl_Position = modelViewProjectionMatrix * vec4(p, 1.);
}
