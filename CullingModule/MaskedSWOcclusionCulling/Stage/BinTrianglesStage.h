#pragma once

#include "MaskedSWOcclusionCullingStage.h"

#include "../../../DataType/Math/AABB.h"
#include "../../../DataType/Math/Triangle.h"

#include "../SWDepthBuffer.h"

namespace culling
{
	
	class BinTrianglesStage : public MaskedSWOcclusionCullingStage
	{
	private:


		/// <summary>
		/// frustum culling in clip space
		/// </summary>
		/// <param name="clipspaceVertexX"></param>
		/// <param name="clipspaceVertexY"></param>
		/// <param name="triangleCullMask"></param>
		EVERYCULLING_FORCE_INLINE void Clipping
		(
			const culling::EVERYCULLING_M256F* const clipspaceVertexX,
			const culling::EVERYCULLING_M256F* const clipspaceVertexY,
			const culling::EVERYCULLING_M256F* const clipspaceVertexZ,
			const culling::EVERYCULLING_M256F* const clipspaceVertexW,
			std::uint32_t& triangleCullMask
		);

		EVERYCULLING_FORCE_INLINE culling::EVERYCULLING_M256F ComputePositiveWMask
		(
			const culling::EVERYCULLING_M256F* const clipspaceVertexW
		);
		
		
		/// <summary>
		/// BackFace Culling
		/// Result is stored in triangleCullMask
		/// </summary>
		/// <param name="screenPixelX"></param>
		/// <param name="screenPixelY"></param>
		/// <param name="ndcSpaceVertexZ"></param>
		/// <param name="triangleCullMask"></param>
		EVERYCULLING_FORCE_INLINE void BackfaceCulling
		(
			culling::EVERYCULLING_M256F* const screenPixelX, 
			culling::EVERYCULLING_M256F* const screenPixelY, 
			std::uint32_t& triangleCullMask
		);

		

		EVERYCULLING_FORCE_INLINE void PassTrianglesToTileBin
		(
			const culling::EVERYCULLING_M256F& pointAScreenPixelPosX,
			const culling::EVERYCULLING_M256F& pointAScreenPixelPosY,
			const culling::EVERYCULLING_M256F& pointANdcSpaceVertexZ,

			const culling::EVERYCULLING_M256F& pointBScreenPixelPosX,
			const culling::EVERYCULLING_M256F& pointBScreenPixelPosY,
			const culling::EVERYCULLING_M256F& pointBNdcSpaceVertexZ,

			const culling::EVERYCULLING_M256F& pointCScreenPixelPosX,
			const culling::EVERYCULLING_M256F& pointCScreenPixelPosY,
			const culling::EVERYCULLING_M256F& pointCNdcSpaceVertexZ,

			const std::uint32_t& triangleCullMask,
			const size_t triangleCountPerLoop,
			const culling::EVERYCULLING_M256I& outBinBoundingBoxMinX,
			const culling::EVERYCULLING_M256I& outBinBoundingBoxMinY,
			const culling::EVERYCULLING_M256I& outBinBoundingBoxMaxX,
			const culling::EVERYCULLING_M256I& outBinBoundingBoxMaxY
		);
		

		/// <summary>
		/// Gather Vertex from VertexList with IndiceList
		/// 
		/// first float of outVerticesX[0] have Triangle1's Point1 X
		/// first float of outVerticesX[1] have Triangle1's Point2 X
		/// first float of outVerticesX[2] have Triangle1's Point3 X
		/// 
		/// second float of outVerticesX[0] have Triangle2's Point1 X
		/// second float of outVerticesX[1] have Triangle2's Point2 X
		/// second float of outVerticesX[2] have Triangle2's Point3 X
		/// </summary>
		/// <param name="vertices"></param>
		/// <param name="vertexIndices"></param>
		/// <param name="indiceCount"></param>
		/// <param name="currentIndiceIndex"></param>
		/// <param name="vertexStrideByte">
		/// how far next vertex point is from current vertex point 
		/// ex) 
		/// 1.0f(Point1_X), 2.0f(Point2_Y), 0.0f(Point3_Z), 3.0f(Normal_X), 3.0f(Normal_Y), 3.0f(Normal_Z),  1.0f(Point1_X), 2.0f(Point2_Y), 0.0f(Point3_Z)
		/// --> vertexStride is 6 * 4(float)
		/// </param>
		/// <param name="fetchTriangleCount"></param>
		/// <param name="outVerticesX"></param>
		/// <param name="outVerticesY"></param>
		/// <param name="triangleCullMask"></param>
		EVERYCULLING_FORCE_INLINE void GatherVertices
		(
			const float* const vertices,
			const size_t verticeCount,
			const std::uint32_t* const vertexIndices,
			const size_t indiceCount, 
			const size_t currentIndiceIndex, 
			const size_t vertexStrideByte, 
			const size_t fetchTriangleCount,
			culling::EVERYCULLING_M256F* outVerticesX, 
			culling::EVERYCULLING_M256F* outVerticesY, 
			culling::EVERYCULLING_M256F* outVerticesZ
		);
		
		/// <summary>
		/// Bin Triangles
		/// </summary>
		/// <param name="vertices"></param>
		/// <param name="vertexIndices"></param>
		/// <param name="indiceCount"></param>
		/// <param name="vertexStrideByte">
		/// how far next vertex point is from current vertex point 
		/// ex) 
		/// 1.0f(Point1_X), 2.0f(Point2_Y), 0.0f(Point3_Z), 3.0f(Normal_X), 3.0f(Normal_Y), 3.0f(Normal_Z),  1.0f(Point1_X), 2.0f(Point2_Y), 0.0f(Point3_Z)
		/// --> vertexStride is 6 * 4(float)
		/// </param>
		/// <param name="modelToClipspaceMatrix"></param>
		EVERYCULLING_FORCE_INLINE void BinTriangles
		(
			culling::EntityBlock* const entityBlock,
			const size_t entityIndex,
			const size_t binnedTriangleListIndex,
			const float* const vertices,
			const size_t verticeCount,
			const std::uint32_t* const vertexIndices,
			const size_t indiceCount,
			const size_t vertexStrideByte,
			const float* const modelToClipspaceMatrix
		);

		void ConvertToPlatformDepth(culling::EVERYCULLING_M256F* const depth);

		//void BinTriangleThreadJob(const size_t cameraIndex);

		/// <summary>
		/// BinTriangle based on front to back ordering
		/// </summary>
		/// <param name="cameraIndex"></param>
		void BinTriangleThreadJobByObjectOrder(const size_t cameraIndex);

	public:

		BinTrianglesStage(MaskedSWOcclusionCulling* mMOcclusionCulling);

		void ResetCullingModule(const unsigned long long currentTickCount) override;

		void CullBlockEntityJob(const size_t cameraIndex, const unsigned long long currentTickCount) override;
		const char* GetCullingModuleName() const override;
	};
}
