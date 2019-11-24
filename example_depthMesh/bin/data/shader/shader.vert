#version 420
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
in vec4 position;
in vec4 color;
in vec4 normal;
in vec2 texcoord;

uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane2;
uniform float offsetY;
uniform float clipZ;

uniform float lds;
out vec4 vPos;
out float vDepth;
out vec2 vTexCoord;

void main() {

    vec4 p = position;
    p.y += offsetY;
    p.z += clipZ;
    vPos = modelViewMatrix * p;
    vDepth = - vPos.z * lds;
    vTexCoord = texcoord;
    gl_Position = modelViewProjectionMatrix * p;
    gl_ClipDistance[0] = dot(p, clipPlane0);
    gl_ClipDistance[1] = dot(p, clipPlane1);
    gl_ClipDistance[2] = dot(p, clipPlane2);
}
