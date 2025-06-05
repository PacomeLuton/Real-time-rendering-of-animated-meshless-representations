#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 A;

layout( set = 0, binding = 0 ) uniform UniformBuffer {
  mat4 model;
  mat4 view;
  mat4 persp;
};

void main()
{

    vec4 minpoint = vec4(A.xy,1/A.z,1.0);
    vec4 maxpoint = vec4(A.xy,1/A.w,1.0);
   

    vec4 a = inverse(persp * view)*minpoint;
    vec4 b = inverse(persp * view)*maxpoint;
    a = a/a.w;
    b = b/b.w;

    float absorbtion = length(b-a);


    outColor = vec4(vec3(max(absorbtion*2,0.1)),0.0);
    //outColor = vec4(vec3(0.5),0);
}