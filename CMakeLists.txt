cmake_minimum_required(VERSION 3.20)
project(EveryCulling)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AVX2 support
if (MSVC)
    add_compile_options(/arch:AVX2)
else()
    add_compile_options(-mavx2)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/DataType)
include_directories(${PROJECT_SOURCE_DIR}/DataType/Math)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/MaskedSWOcclusionCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/MaskedSWOcclusionCulling/Utility)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/ViewFrustumCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/DistanceCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/PreCulling)

# Add subdirectories
add_subdirectory(DataType)
add_subdirectory(CullingModule)

# Add source files for CullingModule
set(EVERYCULLING_MODULE_SOURCES
    EveryCulling.cpp
	EveryCullingProfiler.cpp
	main.cpp
)

# Add the executable
add_executable(EveryCulling ${EVERYCULLING_MODULE_SOURCES})

# Link libraries
target_link_libraries(EveryCulling DataType CullingModule)