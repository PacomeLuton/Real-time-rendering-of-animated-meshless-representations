#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 normal;
layout(location = 1) in vec4 pos;

layout( set = 0, binding = 0 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

void main(){
    vec3 n = normalize(normal.xyz);
    
    vec4 p = pos;
    p /= p.w;

    outColor = vec4(p.z);
    //gl_FragDepth =  p.z / p.w;
    //outColor = vec4(vec3(1),1.0);
    //outColor = vec4((n+1)/2,1.0);
    //outColor = vec4(vec3(dot(n,vec3(0,0,1))),1);

}