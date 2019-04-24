#version 400
uniform sampler2DRect tex;
uniform float zThres;

in vec2 vTexCoord;
out vec4 outputColor;

void main(){

    float z = texture(tex, vTexCoord).z;
    if (z < 0.1) discard;

    float b = 1. - step(zThres, z);

    outputColor = vec4(vec3(b), 1.);

}
