#version 420
uniform int isShadow;
uniform sampler2DRect tex;

in vec4 vPos;
in float vDepth;
in vec2 vTexCoord;

out vec4 outputColor0;
out vec4 outputColor1;
out vec4 outputColor2;

vec3 calcFlatNormal(vec3 posInViewSpace){
    vec3 dx = dFdx(posInViewSpace);
    vec3 dy = dFdy(posInViewSpace);
    vec3 n = normalize(cross(normalize(dx), normalize(dy)));

    return - n;
}

void main() {

    if (isShadow == 1) {
        outputColor0.r = vDepth;
        outputColor0.a = 1.;
    } else {
        vec3 n = calcFlatNormal(vPos.xyz);

        outputColor0 = vec4(pow(texture(tex, vTexCoord * vec2(1280, 720)).rgb, vec3(0.5)), 1.);
        outputColor1 = vPos;
        outputColor2 = vec4(n, vDepth);
    }

}
