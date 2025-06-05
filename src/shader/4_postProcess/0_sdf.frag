#version 450
#extension GL_EXT_debug_printf : enable

#include"../commun.glsl"

layout( location = 1 ) in vec2 frag_uv;
layout( location = 0 ) out vec4 frag_color;

#define PI 3.14159265

layout( set = 0, binding = 1 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

float cmap(vec3 pos){
    return length(pos) - 0.5;
}

void main() {
    //vec4 d = vec4(cos(10*frag_uv.x),sin(10*frag_uv.y),0,1);
    frag_color = vec4(1);

    vec2 uv = frag_uv;
    //vec3 rayDirView = normalize(vec3(uv*2-1, persp[1][1]));
    vec3 rayDirView = normalize(vec3((uv*2-1) / vec2(persp[0][0], persp[1][1]), 1));
    vec3 rayOri = inverse(view*model)[3].xyz;
    vec3 rayDir = normalize(mat3(inverse(view*model)) * rayDirView);
    

    debugPrintfEXT("%v4f\n%v4f\n%v4f\n%v4f\n\n", view[0],view[1],view[2],view[3]);

    //rayOri = vec3(0,0,-1.1);
    //rayDir = normalize(vec3(uv*2-1,0) - rayOri);

    vec2 bv = sphIntersect( rayOri, rayDir, vec4(-0.35,0,0,3.8)/4. );
    if (bv.y <= 0) return;
    frag_color = vec4(1);
    //return;

    float t = max(0,bv.x);
    vec3 pos = rayOri + t*rayDir;
    float d = 100;
    float eps = 0.001;
    for(int i = 0; i < 256 && d > eps; i++){
        pos = rayOri + t*rayDir;
        d = map(pos);
        t += d;
        if (t > bv.y) break;
    }
    
    if (d < eps){
        vec3 n = map_normal(pos);
        frag_color = vec4((n+1)/2,1);
    }

    //frag_color = vec4(1-d.x);
    //frag_color = vec4((d.xyz+1)/2,1);

}