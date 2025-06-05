#version 450
#extension GL_EXT_debug_printf : enable

#include"../commun.glsl"

layout( location = 1 ) in vec2 frag_uv;
layout( location = 0 ) out vec4 frag_color;

#define PI 3.14159265

layout( set = 0, binding = 11 ) uniform sampler3D voxelGrid;
layout( set = 0, binding = 12 ) uniform sampler3D voxelGrid2;

layout( set = 0, binding = 5) uniform guibuffer {
  float time;
  int voxelSize;
};

layout( set = 0, binding = 1 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

float cmap(vec3 q){
  float d1 = textureLod(voxelGrid,(q+1)/2,1).x;
  //float d2 = 10*textureLod(voxelGrid2,(q+1)/2,1).x;
  //if (d1 > 0) return  (d1 + smootherstep(0.1,0.,min(d1/0.019,1))*d2)/4;
  return d1;
}

vec3 cmap_normal(vec3 p){
    float dx = 0.001;
    vec3 n = vec3(0);
    float mp = cmap(p);
    n.x = (mp - cmap(p-vec3(dx,0,0)))/dx;
    n.y = (mp - cmap(p-vec3(0,dx,0)))/dx;
    n.z = (mp - cmap(p-vec3(0,0,dx)))/dx;
    return normalize(n); 
}

void main() {
    //vec4 d = vec4(cos(10*frag_uv.x),sin(10*frag_uv.y),0,1);
    frag_color = vec4(0,0,0,1);

    vec2 uv = frag_uv;
    //vec3 rayDirView = normalize(vec3(uv*2-1, persp[1][1]));
    vec3 rayDirView = normalize(vec3((uv*2-1) / vec2(persp[0][0], persp[1][1]), 1));
    vec3 rayOri = inverse(view*model)[3].xyz;
    vec3 rayDir = normalize(mat3(inverse(view*model)) * rayDirView);
    

    debugPrintfEXT("%v4f\n%v4f\n%v4f\n%v4f\n\n", view[0],view[1],view[2],view[3]);

    //rayOri = vec3(0,0,-1.1);
    //rayDir = normalize(vec3(uv*2-1,0) - rayOri);

    vec2 bv = sphIntersect( rayOri, rayDir, vec4(0,0,0,4.5)/4. );
    //if (bv.y <= 0) return;
    frag_color = vec4(1);
    //return;

    float t = 0; //max(0,bv.x);
    vec3 pos = rayOri + t*rayDir;
    float d = 100;
    float eps = 0.001;

    vec3 v = rayDir;
    float ddx = (v.x < 0) ? -0.01 : 1.01;
    float ddy = (v.y < 0) ? -0.01 : 1.01;
    float ddz = (v.z < 0) ? -0.01 : 1.01;

    for(int i = 0; i < 256 && d > eps; i++){
        pos = rayOri + t*rayDir;
        vec3 qq = (pos+1)/2;

        d = textureLod(voxelGrid,qq,1).x;
        if (d < 0) break;

        vec3 fq = fract(qq*voxelSize);
        float tx = (ddx-fq.x)/(v.x);
        float ty = (ddy-fq.y)/(v.y);
        float tz = (ddz-fq.z)/(v.z);

        float dt = min(min(tx,ty),tz) / voxelSize * 2;

        t += dt;
        //if (t > bv.y) break;
    }
    
    if (d < eps){
        vec3 n = normalize(textureLod(voxelGrid2,(pos+1)/2,1).xyz);;
        frag_color = vec4((n+1)/2,1);
    }

    //frag_color = vec4(1-d.x);
    //frag_color = vec4((d.xyz+1)/2,1);

}