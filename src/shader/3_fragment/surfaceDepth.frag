#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_shader_atomic_float2 : require

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
  float near;
  mat4 invPersp;
};

layout( set = 0, binding = 4 ) buffer oldpos{
  mat4 deform[];
};

layout( set = 0, binding = 5) uniform guibuffer {
  float time;
  vec2 thetaPhi;
};

layout( set = 0, binding = 14, R32F ) uniform image2D customdepth;
//layout( set = 0, binding = 14) uniform writeonly image2D customdepth;
//layout( set = 0, binding = 15 ) uniform sampler2D customdepthR;


void main()
{
    vec2 pixel = (A.xy + 1)/2;
    ivec2 pixelID = ivec2(pixel * vec2(1920,1080));
    float currentDepth = imageLoad(customdepth,pixelID).x;

    //float currentDepth = texture(customdepthR,pixel).x;
    //debugPrintfEXT("%f\n", currentDepth);

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

    if (map(p0) < 0){
      discard;
    }

    for(i = 0; i < 256 && dist < max_dist && s < 1 && abs(t) > stop; i++){
        //q = dir*dist + raystart;                      // on mets à jour la position de notre rayon
        //q = a3*s*s*s + a2*s*s + a1*s + a0;
        q = p0 + s*(p1-p0);
        t = map(q);                                   // on calcul sa distance absolue à la surface
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
      vec3 nq = map_normal(q);
      q = (1-m)*olda.xyz + m*(oldb.xyz);                         // on calcul notre point final de notre rayon

      mat3 repereP = (1-m)*repereA + m*repereB;
      mat4 repereP4 = mat4(vec4(repereP[0],0), vec4(repereP[1],0), vec4(repereP[2],0), vec4(0,0,0,1));

      vec4 p = persp * view * model *  vec4(q,1.0);            // on le repasse dans le repère view, ce qui permet de récupérer la profondeur parcouru
      gl_FragDepth =  p.z / p.w;
      imageStore(customdepth,pixelID,vec4(p.z/p.w));


      vec4 np = transpose(inverse(model * repereP4)) * vec4(nq,0);
      //np = transpose(inverse(view * model * transpose(transfoo[I]))) * vec4(nq,0);

      np = normalize(np);
      outColor = vec4((np.xyz + 1)/2,1.0);
      //outColor = vec4(1e5*(1-color.x),0,0,0);
      //outColor = vec4(I/150000.0);
      //outColor = vec4((outColor.r+outColor.g+outColor.b)/3);
      //debugPrintfEXT("%v3f\n%v3f\n%v3f\n\n",repereA[0],repereA[1],repereA[2]);
      //debugPrintfEXT("%v4f\n",np);
      
      p = model*vec4(q,1.0);
      np = vec4(normalize(cross(dFdx(p.xyz),dFdy(p.xyz))),0);
      //outColor = vec4((np.xyz + 1)/2,1.0);

      vec3 ligthDir = angleToVec(thetaPhi);
    
      //outColor = vec4((normalize(inverse(repereA)*omega)+1)/2,0);
      //outColor = vec4(vec3(abs(dot(np.xyz,ligthDir))),0);
      //outColor = vec4(vec3(dot(np.xyz,np.xyz)),0);
      //outColor = vec4(vec3(p.z),1);

      //imageAtomicMin(customdepth,pixelID,p.z/p.w);
    }else{
      discard;
    }

}