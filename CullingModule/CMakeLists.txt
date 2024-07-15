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
	MaskedSWOcclusionCulling/OccluderListManager.cpp
	MaskedSWOcclusionCulling/SWDepthBuffer.cpp
	MaskedSWOcclusionCulling/Stage/RasterizeOccludersStage.cpp
	MaskedSWOcclusionCulling/Stage/SolveMeshRoleStage.cpp
	MaskedSWOcclusionCulling/Stage/BinTrianglesStage.cpp
	MaskedSWOcclusionCulling/Stage/MaskedSWOcclusionCullingStage.cpp
	MaskedSWOcclusionCulling/Stage/QueryOccludeeStage.cpp
)

# Create a static library for CullingModule
add_library(CullingModule STATIC ${CULLING_MODULE_SOURCES})

# Link DataType library
target_link_libraries(CullingModule DataType)