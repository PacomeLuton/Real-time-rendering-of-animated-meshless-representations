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

layout( set = 0, binding = 5 ) uniform guibuffer {
  float time;
  vec2 thetaPhi;
};


layout( set = 0, binding = 4 ) buffer oldpos{
  mat4 deform[];
};

layout( set = 0, binding = 11 ) uniform sampler3D voxelGrid;
layout( set = 0, binding = 12 ) uniform sampler3D voxelGrid2;

layout( set = 0, binding = 14, R32F ) uniform image2D customdepth;

#define rot(P,A,a) ( mix(A*dot(P,A), P, cos(a) ) + sin(a)*cross(P,A) )
float _o = 1;
vec3 randDir3D2(ivec3 seed) {
 const int width = 10000;
 randstate = seed.x + seed.y * width + seed.z * width*width;
 vec3 h3 = vec3(randf(), randf(), randf());
 vec3 dir_vect = normalize(h3*2.-1.);
    
 vec3 axis = vec3(1.,0.,0.);
    
 //vec3 time_rotated_dir = rot(dir_vect, axis, (dir_vect.x+dir_vect.y+dir_vect.z)*time*7.2);
  vec3 time_rotated_dir = rot(dir_vect, axis, exp2(_o)*time*2);

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

void main()
{
    vec2 pixel = (A.xy + 1)/2;
    ivec2 pixelID = ivec2(pixel * vec2(1920,1080));
    float currentDepth = imageLoad(customdepth,pixelID).x;
    if (A.z > currentDepth) discard;

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


    //on repasse dans le repere du model
    a = inverse(view * model) * a;
    b = inverse(view * model) * b;

    vec4 olda = a;
    vec4 oldb = b;

    a += vec4(da,0);
    b += vec4(db,0);

    //on peut faire un ray marching pour trouver la surface
    vec3 raystart = a.xyz;                             // on part du point a
    vec3 dir = normalize(b.xyz - raystart.xyz);        // on va dans la direction du point b
    float dist = 0;                                    // on commence a une distance 0 du point a 
    float max_dist = length( b.xyz - raystart.xyz);    // on calcul la distance max qu'on peut parcourir avant d'arriver au point b

    int i = 0;                                         // nombre de pas qu'on fait
    float t = 1;                                       // distance temporaire à la surface
    float oldt = 0;
    float stop = 0.001;                                // condition d'arret, si t < stop, c'est qu'on est arrivé au niveau de la surface
    vec3 q = dir*dist + raystart;                      // position courante de notre point
    
    vec3 omega = oldb.xyz - olda.xyz;
    vec3 p0 = a.xyz; vec3 p1 = b.xyz; 

    float s = 0;

    //if (map(p0) < 0){
    //  discard;
    //}

    for(i = 0; i < 256 && dist < max_dist && s < 1 && abs(t) > stop; i++){
        //q = dir*dist + raystart;                      // on mets à jour la position de notre rayon
        //q = a3*s*s*s + a2*s*s + a1*s + a0;
        q = p0 + s*(p1-p0);
        t = cmap(q);
        //dist = dist + abs(t) * 0.2;                   // on fait un pas
        s += abs(t) / (max_dist * 2);
    }


    // on a maintenant fini notre marching
    if(abs(t) < stop){                                  // si on a atteinds une frontiere
      float m = dist / max_dist;                    
      m = s;
      //q = dir*dist + raystart;                         // on calcul notre point final de notre rayon
      //q = a3*s*s*s + a2*s*s + a1*s + a0;
      q = p0 + s*(p1-p0);

      float blend = textureLod(voxelGrid,(q+1)/2,1).x;
      float bl = -(blend-0.019)*10;
      vec4 baseColor = mix(vec4(1,0,0,1),vec4(0,1,0,1),1-bl);

      //vec4 baseColor = (max(0,q.z)/q.z)*vec4(1,0,0,1)+(max(0,-q.z)/(-q.z))*vec4(0,1,0,1);
      vec3 nq = cmap_normal(q);
      //vec3 nq = normalize(textureLod(voxelGrid2,(q+1)/2,1).xyz);
      q = (1-m)*olda.xyz + m*(oldb.xyz);                         // on calcul notre point final de notre rayon

      mat3 repereP = (1-m)*repereA + m*repereB;
      mat4 repereP4 = mat4(vec4(repereP[0],0), vec4(repereP[1],0), vec4(repereP[2],0), vec4(0,0,0,1));

      vec4 p = persp * view * model *  vec4(q,1.0);            // on le repasse dans le repère view, ce qui permet de récupérer la profondeur parcouru
      vec4 np = transpose(inverse(model * repereP4)) * vec4(nq,0);
      //np = transpose(inverse(view * model * transpose(transfoo[I]))) * vec4(nq,0);

      np = normalize(np);
      //outColor = vec4((np.xyz + 1)/2,0.0);
      
      outColor = 1-vec4(vec3(0.5+0.5*max(0,dot(np.xyz,normalize(vec3(0.0,4.1,1.0))))),1)*baseColor;
      outColor = 1-vec4(vec3(0.2+0.8*max(0,dot(np.xyz,normalize(vec3(0.0,-4.1,1.0))))),1)*baseColor;
      //outColor = baseColor;

      gl_FragDepth =  p.z / p.w;
      imageStore(customdepth,pixelID,vec4(p.z/p.w));

                  
    }else{
      discard;
    }

}