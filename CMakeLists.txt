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

set (ROOT_INCLUDE_DIR ${PROJECT_SOURCE_DIR} CACHE STRING "Usually just the ${PROJECT_SOURCE_DIR}")
set (GLFW_INCLUDE_DIR "C:\\Users\\Admin\\Documents\\source\\sdks\\glfw-3.3\\include" CACHE STRING "Path to GLFW include directory")
set (GLM_INCLUDE_DIR "C:\\Users\\Admin\\Documents\\source\\sdks\\glm" CACHE STRING "Path to GLM directory")
set (GLEW_INCLUDE_DIR "C:\\Users\\Admin\\Documents\\source\\sdks\\glew-2.1.0\\include" CACHE STRING "Path to GLEW include directory")

# Include directories
include_directories(${ROOT_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/DataType)
include_directories(${PROJECT_SOURCE_DIR}/DataType/Math)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/MaskedSWOcclusionCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/MaskedSWOcclusionCulling/Utility)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/ViewFrustumCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/DistanceCulling)
include_directories(${PROJECT_SOURCE_DIR}/CullingModule/PreCulling)

set (GLFW_DEBUG_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glfw-3.3\\build_VS2022\\src\\Debug\\glfw3.lib" CACHE STRING "Path to debug GLFW lib file")
set (GLM_DEBUG_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glm\\build\\glm\\Debug\\glm_static.lib" CACHE STRING "Path to debug GLM lib file")
set (GLEW_DEBUG_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glew-2.1.0\\lib\\Debug\\x64\\glew32sd.lib" CACHE STRING "Path to debug GLEW lib file")

set (GLFW_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glfw-3.3\\build_VS2022\\src\\Release\\glfw3.lib" CACHE STRING "Path to GLFW lib file")
set (GLM_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glm\\build\\glm\\Release\\glm_static.lib" CACHE STRING "Path to GLM lib file")
set (GLEW_LIB "C:\\Users\\Admin\\Documents\\source\\sdks\\glew-2.1.0\\lib\\Release\\x64\\glew32sd.lib" CACHE STRING "Path to GLEW lib file")

set (DEBUG_LIB_FILE_PATHS debug ${GLFW_DEBUG_LIB} debug ${GLM_DEBUG_LIB} debug ${GLEW_DEBUG_LIB})

set( ADDITIONAL_LIBRARIES 
    ${DEBUG_LIB_FILE_PATHS}
    optimized ${GLFW_LIB} optimized ${GLM_LIB} optimized ${GLEW_LIB} 
)

#set (LIB_PATHS ${GLFW_LIB_DIR} ${GLM_LIB_DIR} ${GLEW_LIB_DIR})

#link_directories(${LIB_PATHS})

#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /LIBPATH:\""${LIB_PATHS}"\"") 

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

target_link_libraries(EveryCulling 
	${ADDITIONAL_LIBRARIES}
	CullingModule
	OpenGL32.lib
)