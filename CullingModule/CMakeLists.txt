# Add source files for CullingModule
set(CULLING_MODULE_SOURCES
    CullingModule.cpp
    DistanceCulling/DistanceCulling.cpp
    MaskedSWOcclusionCulling/MaskedSWOcclusionCulling.cpp
    MaskedSWOcclusionCulling/Utility/CoverageRasterizer.cpp
    MaskedSWOcclusionCulling/Utility/DepthValueComputer.cpp
    MaskedSWOcclusionCulling/Utility/DepthUtility.cpp
    MaskedSWOcclusionCulling/Utility/RasterizerHelper.cpp
    MaskedSWOcclusionCulling/Utility/clipTriangle.cpp
    MaskedSWOcclusionCulling/Utility/depthBufferTileHelper.cpp
    MaskedSWOcclusionCulling/Utility/triangleSlopeEventGetter.cpp
    MaskedSWOcclusionCulling/Utility/vertexTransformationHelper.cpp
    PreCulling/PreCulling.cpp
    ViewFrustumCulling/ViewFrustumCulling.cpp
)

# Create a static library for CullingModule
add_library(CullingModule STATIC ${CULLING_MODULE_SOURCES})

# Link DataType library
target_link_libraries(CullingModule DataType)