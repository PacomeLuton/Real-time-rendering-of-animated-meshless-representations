#version 450
#extension GL_EXT_debug_printf : enable

#include"../commun.glsl"

struct potage {
    vertexAttribut s[2];
};

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 A;
layout(location = 1) flat in int I;
layout(location = 2) in potage D;

layout( set = 0, binding = 0 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

layout( set = 0, binding = 4 ) buffer oldpos{
  mat4 deform[];
};

layout( set = 0, binding = 11 ) uniform sampler3D voxelGrid;

void main()
{

    vec4 minpoint = vec4(A.xy,A.z,1.0);
    vec4 maxpoint = vec4(A.xy,A.w,1.0);

    vec4 a = inverse(persp)*minpoint;
    vec4 b = inverse(persp)*maxpoint;

    vec3 da = D.s[0].uvw / a.w;
    vec3 db = D.s[1].uvw / b.w;

    mat3 repereA = D.s[0].repere / a.w;
    mat3 repereB = D.s[1].repere / b.w;

    a = a/a.w;
    b = b/b.w;

    vec4 worlda = a;
    vec4 worldb = b;

    //on repasse dans le repere du model
    a = inverse(view * model) * a;
    b = inverse(view * model) * b;

    vec4 olda = a;
    vec4 oldb = b;

    a += vec4(da,0);
    b += vec4(db,0);

    //On peut maintenant faire du marching dans la grille de voxel
    float worldSpaceStep = 0.001;
    if (length(worldb-worlda) > 10) debugPrintfEXT("%i\n",length(worldb-worlda));
    float worldLength = min(length(worldb-worlda),1);
    float localLength = length(b-a);

    vec3 start = a.xyz;
    vec3 end = b.xyz;
    bool hit = false;
    vec3 color = vec3(0);
    
    float t = 0; //position du pas en poseSpace
  
    float worldT = 0;
    for(;worldT <= worldLength; worldT += worldSpaceStep){
        t = min(1,worldT/(worldLength+1e-9)); // position du points dans l'espace
        float nt = min(1,(worldT+worldSpaceStep)/(worldLength+1e-9));
        vec3 pos = (1-t)*start+t*end;
        vec4 voxel = textureLod(voxelGrid,(pos+1)/2,1);
        color += (nt-t)*localLength*voxel.xyz; //la couleur moyenne entre les 2 pas
    }

    // on sauvegarde la couleur  
    //outColor = vec4(localLength*(start+1)/2,1);
    //outColor = vec4(localLength*texture(voxelGrid,vec3(5,5,5)).xyz,1);
 
    outColor = vec4(1.5*color,1);
}