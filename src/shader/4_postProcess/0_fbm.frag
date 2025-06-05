#version 450
#extension GL_EXT_debug_printf : enable

#include"../commun.glsl"

layout( location = 1 ) in vec2 frag_uv;
layout( location = 0 ) out vec4 frag_color;

#define PI 3.14159265

layout( set = 0, binding = 11 ) uniform sampler3D voxelGrid;
layout( set = 0, binding = 12 ) uniform sampler3D voxelGrid2;

layout( set = 0, binding = 5 ) uniform guibuffer {
  float time;
  vec2 thetaPhi;
};

layout( set = 0, binding = 1 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

#define rot(P,A,a) ( mix(A*dot(P,A), P, cos(a) ) + sin(a)*cross(P,A) )
float _o = 1;
vec3 randDir3D2(ivec3 seed) {
 const int width = 10000;
 randstate = seed.x + seed.y * width + seed.z * width*width;
 vec3 h3 = vec3(randf(), randf(), randf());
 vec3 dir_vect = normalize(h3*2.-1.);
    
 vec3 axis = vec3(1.,0.,0.);
    
 //vec3 time_rotated_dir = rot(dir_vect, axis, (dir_vect.x+dir_vect.y+dir_vect.z)*time*7.2);
  vec3 time_rotated_dir = rot(dir_vect, axis, exp2(_o)*2);

 return time_rotated_dir;
}

float cornerValue2(vec3 gridUVW, ivec3 corner) {
    ivec3 cell = ivec3(gridUVW) + corner;
    vec3 offset = gridUVW - cell;
    vec3 gradient = randDir3D2(cell);
    return dot(offset, gradient);
}

float perlin2(ivec3 gridSize, vec3 uvw) {
    vec3 gridUVW = uvw * gridSize;
    vec3 cellUVW = fract(gridUVW);
    return
    smootherstep(
        smootherstep(
            smootherstep(
                cornerValue2(gridUVW, ivec3(0, 0, 0)),
                cornerValue2(gridUVW, ivec3(1, 0, 0)),
                cellUVW.x),
            smootherstep(
                cornerValue2(gridUVW, ivec3(0, 1, 0)),
                cornerValue2(gridUVW, ivec3(1, 1, 0)),
                cellUVW.x),
            cellUVW.y),
        smootherstep(
            smootherstep(
                cornerValue2(gridUVW, ivec3(0, 0, 1)),
                cornerValue2(gridUVW, ivec3(1, 0, 1)),
                cellUVW.x),
            smootherstep(
                cornerValue2(gridUVW, ivec3(0, 1, 1)),
                cornerValue2(gridUVW, ivec3(1, 1, 1)),
                cellUVW.x),
            cellUVW.y),
        cellUVW.z);
}

float fbm2(ivec3 gridSize, int octaves, vec3 uvw) {
    float value = 0;
    float coef = 1;
    for (int o = 0; o < octaves; o++) {
        _o = float(o);
        value += perlin2(gridSize, uvw) * coef;
        gridSize *= 2;
        coef /= 2;
    }
    return value;
}

float cmap(vec3 pos){
  float corps = textureLod(voxelGrid,(pos+1)/2,1).x;
  //return corps;
  if (corps > 0) return  (corps + smootherstep(0.1,0.,min(corps/0.019,1))*(fbm2(ivec3(40), 5, (pos+1)/2)+0.2))/4;
  return (corps + 0.1*(fbm2(ivec3(40), 5, (pos+1)/2)+0.2))/4;
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
    

    //debugPrintfEXT("%v4f\n%v4f\n%v4f\n%v4f\n\n", view[0],view[1],view[2],view[3]);

    //rayOri = vec3(0,0,-1.1);
    //rayDir = normalize(vec3(uv*2-1,0) - rayOri);

    vec2 bv = sphIntersect( rayOri, rayDir, vec4(0,0,0,4.5)/4. );
    if (bv.y <= 0) return;
    frag_color = vec4(1);
    //return;

    float t = max(0,bv.x);
    vec3 pos = rayOri + t*rayDir;
    float d = 100;
    float eps = 0.001;
    for(int i = 0; i < 256 && d > eps; i++){
        pos = rayOri + t*rayDir;
        d = cmap(pos);
        t += abs(d);
        if (t > bv.y) break;
    }
    
    if (d < eps){
        vec3 n = cmap_normal(pos);
        frag_color = vec4((n+1)/2,1);
    }

    //frag_color = vec4(1-d.x);
    //frag_color = vec4((d.xyz+1)/2,1);

}