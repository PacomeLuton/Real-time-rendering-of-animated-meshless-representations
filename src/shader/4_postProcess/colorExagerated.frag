#version 450
layout( set = 0, binding = 0 ) uniform sampler2D density;

layout( set = 0, binding = 2 ) uniform sampler2D env;

layout( location = 1 ) in vec2 frag_uv;
layout( location = 0 ) out vec4 frag_color;

#define PI 3.14159265


void main() {

    vec4 d = texture(density,vec2(frag_uv.x, frag_uv.y)).xyzw;

    //frag_color = vec4(1-d.x);
    //frag_color = vec4((d.xyz+1)/2,1);
    frag_color = vec4(pow(d.xyz,1+0*vec3(0.4,0.4,0.2)),1);
    frag_color = vec4(exp(-0.7*d.xyz),1);
}