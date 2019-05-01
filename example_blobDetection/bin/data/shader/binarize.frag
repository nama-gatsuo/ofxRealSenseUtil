#version 400
uniform sampler2DRect input;
uniform float zThres;

in vec2 vTexCoord;
out vec4 outputColor;

void main(){

    vec2 res = textureSize(input);

    float z = texture(input, vTexCoord / (vec2(1280, 720) / res)).z;
    if (z < 0.1) discard;

    float b = 1. - step(zThres, z);

    outputColor = vec4(vec3(b), 1.);

}
