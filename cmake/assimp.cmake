
include(FetchContent)
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG 79d451b
)
FetchContent_MakeAvailable(assimp)