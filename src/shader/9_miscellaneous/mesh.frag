#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 normal;
layout(location = 1) in vec4 pos;

layout( set = 0, binding = 0 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

layout( set = 0, binding = 18) uniform cam {
  mat4 camview;
};

layout( set = 0, binding = 19 ) uniform sampler2D shadows;

void main(){
    vec4 p = inverse(persp)*pos;
    p /= p.w;
    p = inverse(view) * p;
    p = persp*camview*p;
    float oz = p.z;
    p /= p.w;

    vec4 d = texture(shadows,vec2(p.x/2+.5, p.y/2+.5)).xyzw;


    float ix = p.x/2+.5;
    float iy = p.y/2+.5;

    bool t = ix > 0 && ix < 1 && iy > 0 && iy < 1 && oz > 0;
    //outColor = vec4(pow(p.z,10));
    //outColor = vec4(pow(d.z,10));


    vec3 n = normalize(normal.xyz);
    
    //gl_FragDepth =  p.z / p.w;
    //outColor = vec4(vec3(1),1.0);
    outColor = vec4((n+1)/2,1.0);

    float phong = .8*dot(n.xyz,normalize(vec3(0.0,-4.1*1.4,2.5*1.2)));
    outColor = 0.2 * vec4(1,1,1,1) + max(phong,0) * vec4(1,1,0.2,1) + max(-phong,0)*vec4(0,0,0.2,1); 

    if(d.z+0.00001 < p.z && t) {
      outColor *= 0.4;
    }

    outColor = 1-outColor;
    //outColor = vec4(vec3(dot(n,vec3(0,0,1))),1);

}