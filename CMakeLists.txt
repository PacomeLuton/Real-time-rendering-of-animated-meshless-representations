cmake_minimum_required (VERSION 3.20)
project( "Interval" )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libigl)
igl_include(copyleft core)
igl_include(copyleft tetgen)
igl_include(stb)

include(assimp)


add_subdirectory(LavaCake)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/objects DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/1_spaceDeformation)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/2_intervalShading)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tetMeshGen)

add_compile_definitions(PROJECT_ROOT="${CMAKE_CURRENT_BINARY_DIR}/")

clearShader()
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/0_coarse/loadVertices.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse/loadVertices.comp.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/0_coarse/loadTets.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse/loadTets.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/0_coarse/uniformQuadALL.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse/uniformQuadALL.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/0_coarse/uniformQuadSurface.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse/uniformQuadSurface.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/0_coarse/singleTet.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/0_coarse/singleTet.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)

addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/1_spaceDeformation/globalImplicite.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/1_spaceDeformation/globalImplicite.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/1_spaceDeformation/rig.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/1_spaceDeformation/rig.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)

addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/2_intervalShading/interval_noclip.mesh ${CMAKE_CURRENT_BINARY_DIR}/shaders/2_intervalShading/interval_noclip.mesh.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/2_intervalShading/clipTet.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/2_intervalShading/clipTet.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)

addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/worldSpaceTransparency.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/worldSpaceTransparency.frag.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/surface.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/surface.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/surfaceDepth.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/surfaceDepth.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelsdf.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelsdf.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelsdfR.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelsdfR.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelsdfZ.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelsdfZ.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelsdfD.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelsdfD.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelsdfShade.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelsdfShade.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelVisible.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelVisible.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxel.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxel.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/3_fragment/voxelColorFromSDF.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/3_fragment/voxelColorFromSDF.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)

addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/default.vert ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/default.vert.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/identity.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/identity.frag.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/identity1.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/identity1.frag.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/0_voxel.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/0_voxel.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/0_voxelSdf.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/0_voxelSdf.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/0_sdf.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/0_sdf.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/0_fbm.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/0_fbm.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/0_transparant.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/0_transparant.frag.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)

addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/4_postProcess/colorExagerated.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/4_postProcess/colorExagerated.frag.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/implicite2voxel.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/implicite2voxel.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/voxelGeneration.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/voxelGeneration.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/voxelGenerationSDF.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/voxelGenerationSDF.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/voxelGenerationSDF2.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/voxelGenerationSDF2.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/voxelVisible.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/voxelVisible.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/voxelFBM.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/voxelFBM.comp.spv spv1.4 ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/commun.glsl)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/sortingTest.comp ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/sortingTest.comp.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/mesh.mesh ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/mesh.mesh.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/mesh.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/mesh.frag.spv spv1.4)
addShader(${CMAKE_CURRENT_SOURCE_DIR}/src/shader/9_miscellaneous/meshZ.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/9_miscellaneous/meshZ.frag.spv spv1.4)



add_executable( generateShellFromSDF  "./src/tetMeshGen/generateShellFromSDF.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(generateShellFromSDF)
target_link_libraries( generateShellFromSDF LavaCake  )

add_executable( generateTetMesh  "./src/tetMeshGen/generateTetMesh.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(generateTetMesh)
target_link_libraries( generateTetMesh igl_copyleft::tetgen igl::stb igl_copyleft::core)


add_executable( 0_ArloSDF  "./src/0_ArloSDF.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(0_ArloSDF)
target_link_libraries( 0_ArloSDF LavaCake )

add_executable( 1_AnimatedCage  "./src/1_AnimatedCage.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(1_AnimatedCage)
target_link_libraries( 1_AnimatedCage LavaCake assimp)

add_executable( 2_AnimatedSDF  "./src/2_AnimatedSDF.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(2_AnimatedSDF)
target_link_libraries( 2_AnimatedSDF LavaCake assimp)

add_executable( 3_AnimatedVoxel  "./src/3_AnimatedVoxel.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(3_AnimatedVoxel)
target_link_libraries( 3_AnimatedVoxel LavaCake assimp)

add_executable( 4_AnimatedFBM  "./src/4_AnimatedFBM.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(4_AnimatedFBM)
target_link_libraries( 4_AnimatedFBM LavaCake assimp)

add_executable( 5_AnimatedDynamicFBM  "./src/5_AnimatedDynamicFBM.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(5_AnimatedDynamicFBM )
target_link_libraries( 5_AnimatedDynamicFBM  LavaCake assimp)

add_executable( 6_AnimatedDensityField  "./src/6_AnimatedDensityField.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(6_AnimatedDensityField )
target_link_libraries( 6_AnimatedDensityField  LavaCake assimp)

add_executable( 7_SelfIntersection  "./src/7_SelfIntersection.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(7_SelfIntersection )
target_link_libraries( 7_SelfIntersection  LavaCake assimp)

add_executable( 8_IntegrationWithTriangle  "./src/8_IntegrationWithTriangle.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(8_IntegrationWithTriangle )
target_link_libraries( 8_IntegrationWithTriangle  LavaCake assimp)

add_executable( 9_WiggleArlo  "./src/9_WiggleArlo.cpp")
set( CMAKE_CXX_FLAGS_DEBUG_INIT "-fstack-protector" )
AutoSPIRV(9_WiggleArlo)
target_link_libraries( 9_WiggleArlo LavaCake )