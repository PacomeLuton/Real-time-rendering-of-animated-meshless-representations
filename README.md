# Real-time rendering of animated meshless representations


## Dependency
This project can be run a un Ubuntu 22, your GPU need to be mesh shader compatible. To compile, you need the following dependency:
+ C++20 compiler
+ CMake
+ Vulkan SDK (1.3.239 or newer)
+ GLFW3

To install apt depency you can run: `sudo apt install build-essential libvulkan-dev glslc libglfw3-dev libxinerama-dev libxcursor-dev libxi-dev`


## Compiling 

Pull all the submodules and compile with the following command
```
git submodule update --init --recursive
cmake -B build
cd build
make -j
```

## Testing

Then you can run the different examples from the paper:
+ 0_ArloSDF
+ 1_AnimatedCage
+ 2_AnimatedSDF
+ 3_AnimatedVoxel
+ 4_AnimatedFBM
+ 5_AnimatedDynamicFBM
+ 6_AnimatedDensityField
+ 7_SelfIntersection
+ 8_IntegrationWithTriangle
+ 9_WiggleArlo

## Citing

https://hal.science/hal-05095359v1
