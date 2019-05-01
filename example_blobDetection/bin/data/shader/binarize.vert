#version 400
uniform mat4 modelViewProjectionMatrix; // oF Default
in vec4 position; // oF Default
in vec2 texcoord; // oF Default

out vec2 vTexCoord;

void main(){
    vec2 p = position.xy * 2.;
    vTexCoord = p;
    gl_Position = modelViewProjectionMatrix * vec4(p, 0., 1.);
}
