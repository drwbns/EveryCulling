﻿#include "RasterizeOccludersStage.h"

#include "../MaskedSWOcclusionCulling.h"
#include "../Utility/CoverageRasterizer.h"
#include "../Utility/DepthValueComputer.h"
#include "../Utility/RasterizerHelper.h"
#include "../Utility/triangleSlopeEventGetter.h"

void culling::RasterizeOccludersStage::UpdateHierarchicalDepthBuffer()
{
	/*
			// Discard working layer heuristic
			dist1t = tile.zMax1 - tri.zMax
			dist01 = tile.zMax0 - tile.zMax1
			if (dist1t > dist01)
				tile.zMax1 = 0
				tile.mask = 0
			// Merge current triangle into working layer
			tile.zMax1 = max(tile.zMax1, tri.zMax)
			tile.mask |= tri.coverageMask
			// Overwrite ref. layer if working layer full
			if (tile.mask == ~0)
				tile.zMax0 = tile.zMax1
				tile.zMax1 = 0
				tile.mask = 0
			*/
}

void culling::RasterizeOccludersStage::ComputeTrianglesDepthValueInTile()
{
}


culling::M256I culling::RasterizeOccludersStage::ShuffleCoverageMask(const culling::M256I& coverageMask) const
{
	static const culling::M256I shuffleMask
	=
	_mm256_setr_epi8
	(
		0, 4, 8, 12,
		1, 5, 9, 13,
		2, 6, 10, 14,
		3, 7, 11, 15,

		0, 4, 8, 12,
		1, 5, 9, 13,
		2, 6, 10, 14,
		3, 7, 11, 15
	);

	return _mm256_shuffle_epi8(coverageMask, shuffleMask);
}

void culling::RasterizeOccludersStage::RasterizeBinnedTriangles
(
	const size_t cameraIndex, 
	culling::Tile* const tile
)
{
	assert(tile != nullptr);

	const culling::Vec2 tileOriginPoint{ static_cast<float>(tile->GetLeftBottomTileOrginX()), static_cast<float>(tile->GetLeftBottomTileOrginY()) };

	/*
	for (size_t triangleIndex = 0; triangleIndex < tile->mBinnedTriangles.mCurrentTriangleCount; triangleIndex++)
	{
		culling::TwoDTriangle twoDTriangle;
		twoDTriangle.Points[0] = { tile->mBinnedTriangles.VertexX[0][triangleIndex], tile->mBinnedTriangles.VertexY[0][triangleIndex] };
		twoDTriangle.Points[1] = { tile->mBinnedTriangles.VertexX[1][triangleIndex], tile->mBinnedTriangles.VertexY[1][triangleIndex] };
		twoDTriangle.Points[2] = { tile->mBinnedTriangles.VertexX[2][triangleIndex], tile->mBinnedTriangles.VertexY[2][triangleIndex] };
		culling::M256I result = culling::CoverageRasterizer::FillTriangle(tileOriginPoint, twoDTriangle);
		tile->mHizDatas.l1CoverageMask = _mm256_or_si256(tile->mHizDatas.l1CoverageMask, result);
	}
	*/
	
	for (size_t triangleBatchIndex = 0; triangleBatchIndex < tile->mBinnedTriangles.mCurrentTriangleCount; triangleBatchIndex += 8)
	{
		const size_t triangleCount = MIN(8, tile->mBinnedTriangles.mCurrentTriangleCount - triangleBatchIndex);
		assert(triangleCount <= 8);

		TwoDTriangle twoDTriangle;

		culling::M256F& TriPointA_X = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexX[0][triangleBatchIndex]));
		culling::M256F& TriPointA_Y = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexY[0][triangleBatchIndex]));
		culling::M256F& TriPointA_Z = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexZ[0][triangleBatchIndex]));
		culling::M256F& TriPointB_X = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexX[1][triangleBatchIndex]));
		culling::M256F& TriPointB_Y = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexY[1][triangleBatchIndex]));
		culling::M256F& TriPointB_Z = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexZ[1][triangleBatchIndex]));
		culling::M256F& TriPointC_X = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexX[2][triangleBatchIndex]));
		culling::M256F& TriPointC_Y = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexY[2][triangleBatchIndex]));
		culling::M256F& TriPointC_Z = *reinterpret_cast<culling::M256F*>(&(tile->mBinnedTriangles.VertexZ[2][triangleBatchIndex]));

		Sort_8_2DTriangles(TriPointA_X, TriPointA_Y, TriPointB_X, TriPointB_Y, TriPointC_X, TriPointC_Y);

		culling::M256F LEFT_MIDDLE_POINT_X;
		culling::M256F LEFT_MIDDLE_POINT_Y;
		culling::M256F LEFT_MIDDLE_POINT_Z;

		culling::M256F RIGHT_MIDDLE_POINT_X;
		culling::M256F RIGHT_MIDDLE_POINT_Y;
		culling::M256F RIGHT_MIDDLE_POINT_Z;

		// split triangle
		culling::rasterizerHelper::GetMiddlePointOfTriangle
		(
			TriPointA_X,
			TriPointA_Y,
			TriPointA_Z,

			TriPointB_X,
			TriPointB_Y,
			TriPointB_Z,

			TriPointC_X,
			TriPointC_Y,
			TriPointC_Z,

			LEFT_MIDDLE_POINT_X,
			LEFT_MIDDLE_POINT_Y,
			LEFT_MIDDLE_POINT_Z,

			RIGHT_MIDDLE_POINT_X,
			RIGHT_MIDDLE_POINT_Y,
			RIGHT_MIDDLE_POINT_Z
		);

		culling::M256I LeftSlopeEventOfBottomFlatTriangle[8];
		culling::M256I RightSlopeEventOfBottomFlatTriangle[8];

		culling::M256I LeftSlopeEventOfTopFlatTriangle[8];
		culling::M256I RightSlopeEventOfTopFlatTriangle[8];

		culling::triangleSlopeHelper::GatherBottomFlatTriangleSlopeEvent
		(
			triangleCount,
			tileOriginPoint,
			LeftSlopeEventOfBottomFlatTriangle,
			RightSlopeEventOfBottomFlatTriangle,

			TriPointA_X,
			TriPointA_Y,

			LEFT_MIDDLE_POINT_X,
			LEFT_MIDDLE_POINT_Y,

			RIGHT_MIDDLE_POINT_X,
			RIGHT_MIDDLE_POINT_Y
		);

		culling::triangleSlopeHelper::GatherTopFlatTriangleSlopeEvent
		(
			triangleCount,
			tileOriginPoint,
			LeftSlopeEventOfTopFlatTriangle,
			RightSlopeEventOfTopFlatTriangle,
			
			LEFT_MIDDLE_POINT_X,
			LEFT_MIDDLE_POINT_Y,

			RIGHT_MIDDLE_POINT_X,
			RIGHT_MIDDLE_POINT_Y,

			TriPointC_X,
			TriPointC_Y
		);

		culling::M256I CoverageMask[8];
		culling::M256F subTileMaxDepth[8];

		{
			{
				culling::M256I Result1[8], Result2[8];
				culling::CoverageRasterizer::FillBottomFlatTriangleBatch
				(
					triangleCount,
					Result1,
					tileOriginPoint,

					LeftSlopeEventOfBottomFlatTriangle,
					RightSlopeEventOfBottomFlatTriangle,
					LEFT_MIDDLE_POINT_Y
				);


				culling::CoverageRasterizer::FillTopFlatTriangleBatch
				(
					triangleCount,
					Result2,
					tileOriginPoint,

					LeftSlopeEventOfTopFlatTriangle,
					RightSlopeEventOfTopFlatTriangle,
					LEFT_MIDDLE_POINT_Y
				);

				for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
				{
					CoverageMask[triangleIndex] = _mm256_or_si256(Result1[triangleIndex], Result2[triangleIndex]);
				}
			}

			
			for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
			{
				CoverageMask[triangleIndex] = ShuffleCoverageMask(CoverageMask[triangleIndex]);
			}

			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			//
			// --> 256bit
			//
			//
			//
			// 0 : CoverageMask ( 0 ~ 32 )
			// 1 : CoverageMask ( 32 ~ 64 )
			// 2 : CoverageMask ( 64 ~ 96 )
			// 3 : CoverageMask ( 96 ~ 128 )
			// 4 : CoverageMask ( 128 ~ 160 )
			// 5 : CoverageMask ( 160 ~ 192 )
			// 6 : CoverageMask ( 192 ~ 224 )
			// 7 : CoverageMask ( 224 ~ 256 )

		}


		

		{
		

			culling::M256F _subTileMaxDepth1[8]; 
			culling::M256F _subTileMaxDepth2[8];

			culling::DepthValueComputer::ComputeFlatBottomDepthValue
			(
				triangleCount,
				DepthValueComputer::eDepthType::MaxDepth,
				_subTileMaxDepth1,
				tileOriginPoint.x,
				tileOriginPoint.y,

				TriPointA_X,
				TriPointA_Y,
				TriPointA_Z,

				LEFT_MIDDLE_POINT_X,
				LEFT_MIDDLE_POINT_Y,
				LEFT_MIDDLE_POINT_Z,

				RIGHT_MIDDLE_POINT_X,
				RIGHT_MIDDLE_POINT_Y,
				RIGHT_MIDDLE_POINT_Z,

				LeftSlopeEventOfBottomFlatTriangle,
				RightSlopeEventOfBottomFlatTriangle
			);


			culling::DepthValueComputer::ComputeFlatTopDepthValue
			(
				triangleCount,
				DepthValueComputer::eDepthType::MaxDepth,
				_subTileMaxDepth2,
				tileOriginPoint.x,
				tileOriginPoint.y,

				LEFT_MIDDLE_POINT_X,
				LEFT_MIDDLE_POINT_Y,
				LEFT_MIDDLE_POINT_Z,

				RIGHT_MIDDLE_POINT_X,
				RIGHT_MIDDLE_POINT_Y,
				RIGHT_MIDDLE_POINT_Z,

				TriPointC_X,
				TriPointC_Y,
				TriPointC_Z,

				LeftSlopeEventOfTopFlatTriangle,
				RightSlopeEventOfTopFlatTriangle
			);

			

			for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
			{
				subTileMaxDepth[triangleIndex] = _mm256_max_ps(_subTileMaxDepth1[triangleIndex], _subTileMaxDepth2[triangleIndex]);
			}
			

			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 44444444 55555555 66666666 77777777
			// 
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			// 00000000 11111111 22222222 33333333
			//
			// --> 256bit
			//
			//
			//
			// 0 : subTileMaxDepth ( 0 ~ 32 )
			// 1 : subTileMaxDepth ( 32 ~ 64 )
			// 2 : subTileMaxDepth ( 64 ~ 96 )
			// 3 : subTileMaxDepth ( 96 ~ 128 )
			// 4 : subTileMaxDepth ( 128 ~ 160 )
			// 5 : subTileMaxDepth ( 160 ~ 192 )
			// 6 : subTileMaxDepth ( 192 ~ 224 )
			// 7 : subTileMaxDepth ( 224 ~ 256 )


			
		}


		
		for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
		{
			//tile->mHizDatas.L0MaxDepthValue = _mm256_min_ps(tile->mHizDatas.L0MaxDepthValue, subTileMaxDepth[triangleIndex]);

			
			tile->mHizDatas.L1MaxDepthValue = _mm256_max_ps(tile->mHizDatas.L1MaxDepthValue, subTileMaxDepth[triangleIndex]);
			tile->mHizDatas.l1CoverageMask = _mm256_or_si256(tile->mHizDatas.l1CoverageMask, CoverageMask[triangleIndex]);


			const culling::M256I coverageMaskFullByOne = _mm256_cmpeq_epi32(tile->mHizDatas.l1CoverageMask, _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFF));

			tile->mHizDatas.L0MaxDepthValue = _mm256_blendv_ps(tile->mHizDatas.L0MaxDepthValue, _mm256_min_ps(tile->mHizDatas.L0MaxDepthValue, tile->mHizDatas.L1MaxDepthValue), *reinterpret_cast<const culling::M256F*>(&coverageMaskFullByOne));
			tile->mHizDatas.L1MaxDepthValue = _mm256_blendv_ps(tile->mHizDatas.L1MaxDepthValue, _mm256_setzero_ps(), *reinterpret_cast<const culling::M256F*>(&coverageMaskFullByOne));

			const culling::M256F coverageMaskBlendResult = _mm256_blendv_ps(*reinterpret_cast<const culling::M256F*>(&tile->mHizDatas.l1CoverageMask), _mm256_setzero_ps(), *reinterpret_cast<const culling::M256F*>(&coverageMaskFullByOne));
			tile->mHizDatas.l1CoverageMask = *reinterpret_cast<const culling::M256I*>(&coverageMaskBlendResult);
			

		}

		// algo : if coverage mask is full, overrite tile->mHizDatas.L1MaxDepthValue to tile->mHizDatas.lMaxDepthValue and clear coverage mask
	}
	
	


	
#ifdef DEBUG_CULLING
	const culling::M256I test
		=
		_mm256_setr_epi8
		(
			0, 4, 8, 12,
			1, 5, 9, 13,
			2, 6, 10, 14,
			3, 7, 11, 15,
			0, 4, 8, 12,
			1, 5, 9, 13,
			2, 6, 10, 14,
			3, 7, 11, 15
		);

	const culling::M256I correctTestResult
		=
		_mm256_setr_epi8
		(
			0, 1, 2, 3,
			4, 5, 6, 7,
			8, 9, 10, 11,
			12, 13, 14, 15,
			0, 1, 2, 3,
			4, 5, 6, 7,
			8, 9, 10, 11,
			12, 13, 14, 15
		);
	
	const culling::M256I testResult = ShuffleCoverageMask(test);

	assert(_mm256_testc_si256(correctTestResult, testResult) == 1);
#endif
	
}

culling::Tile* culling::RasterizeOccludersStage::GetNextDepthBufferTile(const size_t cameraIndex)
{
	culling::Tile* nextDepthBufferTile = nullptr;

	const size_t currentTileIndex = mFinishedTileCount[cameraIndex].fetch_add(1, std::memory_order_seq_cst);

	const size_t tileCount = mMaskedOcclusionCulling->mDepthBuffer.GetTileCount();

	if(currentTileIndex < tileCount)
	{
		nextDepthBufferTile = mMaskedOcclusionCulling->mDepthBuffer.GetTile(currentTileIndex);
	}

	return nextDepthBufferTile;
}

culling::RasterizeOccludersStage::RasterizeOccludersStage(MaskedSWOcclusionCulling* mOcclusionCulling)
	: MaskedSWOcclusionCullingStage{ mOcclusionCulling }
{
}

void culling::RasterizeOccludersStage::ResetCullingModule()
{
	MaskedSWOcclusionCullingStage::ResetCullingModule();

	for (std::atomic<size_t>& atomicVal : mFinishedTileCount)
	{
		atomicVal.store(0, std::memory_order_relaxed);
	}
}

void culling::RasterizeOccludersStage::CullBlockEntityJob(const size_t cameraIndex)
{
	const culling::Tile* const endTile = mMaskedOcclusionCulling->mDepthBuffer.GetTiles() + mMaskedOcclusionCulling->mDepthBuffer.GetTileCount();

	while(true)
	{
		culling::Tile* const nextTile = GetNextDepthBufferTile(cameraIndex);

		if(nextTile != nullptr)
		{
			if (nextTile->mBinnedTriangles.mCurrentTriangleCount > 0)
			{
				RasterizeBinnedTriangles(cameraIndex, nextTile);
			}
		}
		else
		{
			break;
		}
	}

}

