#include "ViewFrustumCulling.h"

#include <cassert>

#include "../../DataType/Math/Common.h"
#include "../../EveryCulling.h"


culling::ViewFrustumCulling::ViewFrustumCulling(EveryCulling* frotbiteCullingSystem)
	: CullingModule{ frotbiteCullingSystem }
{

}

// TODO : Implement AV2 ( _m256f ) version.
void culling::ViewFrustumCulling::DoViewFrustumCulling
(
	const size_t cameraIndex,
	culling::EntityBlock* const entityBlock
)
{
	assert(entityBlock->mCurrentEntityCount != 0);
	

	const Vec4* frustumPlane = mSIMDFrustumPlanes[cameraIndex].mFrustumPlanes;

	assert(entityBlock->mCurrentEntityCount != 0);
	for (size_t entityIndex = 0; entityIndex < entityBlock->mCurrentEntityCount ; entityIndex = entityIndex + 2)
	{
		if ( (entityBlock->GetIsCulled(entityIndex, cameraIndex) == false) || ((entityIndex + 1 < entityBlock->mCurrentEntityCount) && entityBlock->GetIsCulled(entityIndex + 1, cameraIndex) == false) )
		{

#ifdef EVERYCULLING_DEBUG_CULLING
			if(entityBlock->GetIsCulled(entityIndex, cameraIndex) == false)
			{
				assert(entityBlock->mWorldPositionAndWorldBoundingSphereRadius[entityIndex].GetBoundingSphereRadius() >= 0.0f);
			}
			if ((entityIndex + 1 < entityBlock->mCurrentEntityCount) && entityBlock->GetIsCulled(entityIndex + 1, cameraIndex) == false)
			{
				assert(entityBlock->mWorldPositionAndWorldBoundingSphereRadius[entityIndex + 1].GetBoundingSphereRadius() >= 0.0f);
			}
#endif

			const char result = CheckInFrustumSIMDWithTwoPoint(frustumPlane, entityBlock->mWorldPositionAndWorldBoundingSphereRadius + entityIndex);
			// if first low bit has 1 value, Pos A is In Frustum
			// if second low bit has 1 value, Pos A is In Frustum

			

			entityBlock->UpdateIsCulled(entityIndex, cameraIndex, !(bool)(result & 1));
			entityBlock->UpdateIsCulled(entityIndex + 1, cameraIndex, !(bool)(result & 2));
		}
	}
	
}

void culling::ViewFrustumCulling::CullBlockEntityJob
(
	const size_t cameraIndex, const unsigned long long currentTickCount
)
{

	
	while(true)
	{
		culling::EntityBlock* const nextEntityBlock = GetNextEntityBlock(cameraIndex);

		if(nextEntityBlock != nullptr)
		{
			DoViewFrustumCulling(cameraIndex, nextEntityBlock);
			DebugOutputCullingResult(cameraIndex, nextEntityBlock);  // Add this line
		}
		else
		{
			break;
		}
	}
	
	
}

const char* culling::ViewFrustumCulling::GetCullingModuleName() const
{
	return "ViewFrustumCulling";
}

EVERYCULLING_FORCE_INLINE char culling::ViewFrustumCulling::CheckInFrustumSIMDWithTwoPoint
(
	const Vec4* eightPlanes,
	const Position_BoundingSphereRadius* twoPoint
)
{
	//We can't use culling::EVERYCULLING_M256F. because two twoPoint isn't aligned to 32 byte

	const culling::EVERYCULLING_M128F* const m128f_eightPlanes = reinterpret_cast<const culling::EVERYCULLING_M128F*>(eightPlanes); // x of plane 0, 1, 2, 3  and y of plane 0, 1, 2, 3 
	const culling::EVERYCULLING_M128F* const m128f_2Point = reinterpret_cast<const culling::EVERYCULLING_M128F*>(twoPoint);

	const culling::EVERYCULLING_M128F posA_xxxx = EVERYCULLING_M128F_REPLICATE(m128f_2Point[0], 0); // xxxx of first twoPoint and xxxx of second twoPoint
	const culling::EVERYCULLING_M128F posA_yyyy = EVERYCULLING_M128F_REPLICATE(m128f_2Point[0], 1); // yyyy of first twoPoint and yyyy of second twoPoint
	const culling::EVERYCULLING_M128F posA_zzzz = EVERYCULLING_M128F_REPLICATE(m128f_2Point[0], 2); // zzzz of first twoPoint and zzzz of second twoPoint

	const culling::EVERYCULLING_M128F posA_rrrr = _mm_or_ps(culling::EVERYCULLING_M128F_ADD(EVERYCULLING_M128F_REPLICATE(m128f_2Point[0], 3), _mm_set1_ps(EVERYCULLING_BOUNDING_SPHRE_RADIUS_MARGIN)), _mm_set1_ps(-0.0f)); // rrrr of first twoPoint and rrrr of second twoPoint

	culling::EVERYCULLING_M128F dotPosA = culling::EVERYCULLING_M128F_MUL_AND_ADD(posA_zzzz, m128f_eightPlanes[2], m128f_eightPlanes[3]);
	dotPosA = culling::EVERYCULLING_M128F_MUL_AND_ADD(posA_yyyy, m128f_eightPlanes[1], dotPosA);
	dotPosA = culling::EVERYCULLING_M128F_MUL_AND_ADD(posA_xxxx, m128f_eightPlanes[0], dotPosA); // dot Pos A with Plane 0, dot Pos A with Plane 1, dot Pos A with Plane 2, dot Pos A with Plane 3

	const culling::EVERYCULLING_M128F posB_xxxx = EVERYCULLING_M128F_REPLICATE(m128f_2Point[1], 0); // xxxx of first twoPoint and xxxx of second twoPoint
	const culling::EVERYCULLING_M128F posB_yyyy = EVERYCULLING_M128F_REPLICATE(m128f_2Point[1], 1); // yyyy of first twoPoint and yyyy of second twoPoint
	const culling::EVERYCULLING_M128F posB_zzzz = EVERYCULLING_M128F_REPLICATE(m128f_2Point[1], 2); // zzzz of first twoPoint and zzzz of second twoPoint

	const culling::EVERYCULLING_M128F posB_rrrr = _mm_or_ps(culling::EVERYCULLING_M128F_ADD(EVERYCULLING_M128F_REPLICATE(m128f_2Point[1], 3), _mm_set1_ps(EVERYCULLING_BOUNDING_SPHRE_RADIUS_MARGIN)), _mm_set1_ps(-0.0f));// rrrr of first twoPoint and rrrr of second twoPoint

	culling::EVERYCULLING_M128F dotPosB = culling::EVERYCULLING_M128F_MUL_AND_ADD(posB_zzzz, m128f_eightPlanes[2], m128f_eightPlanes[3]);
	dotPosB = culling::EVERYCULLING_M128F_MUL_AND_ADD(posB_yyyy, m128f_eightPlanes[1], dotPosB);
	dotPosB = culling::EVERYCULLING_M128F_MUL_AND_ADD(posB_xxxx, m128f_eightPlanes[0], dotPosB); // dot Pos B with Plane 0, dot Pos B with Plane 1, dot Pos B with Plane 2, dot Pos B with Plane 3

	//https://software.intel.com/sites/landingpage/IntrinsicsGuide/#expand=69,124,4167,4167,447,447,3148,3148&techs=SSE,SSE2,SSE3,SSSE3,SSE4_1,SSE4_2,AVX&text=insert
	const culling::EVERYCULLING_M128F posAB_xxxx = _mm_shuffle_ps(m128f_2Point[0], m128f_2Point[1], EVERYCULLING_SHUFFLEMASK(0, 0, 0, 0)); // x of twoPoint[0] , x of twoPoint[0], x of twoPoint[1] , x of twoPoint[1]
	const culling::EVERYCULLING_M128F posAB_yyyy = _mm_shuffle_ps(m128f_2Point[0], m128f_2Point[1], EVERYCULLING_SHUFFLEMASK(1, 1, 1, 1)); // y of twoPoint[0] , y of twoPoint[0], y of twoPoint[1] , y of twoPoint[1]
	const culling::EVERYCULLING_M128F posAB_zzzz = _mm_shuffle_ps(m128f_2Point[0], m128f_2Point[1], EVERYCULLING_SHUFFLEMASK(2, 2, 2, 2)); // z of twoPoint[0] , z of twoPoint[0], z of twoPoint[1] , z of twoPoint[1]

	culling::EVERYCULLING_M128F posAB_rrrr = _mm_shuffle_ps(m128f_2Point[0], m128f_2Point[1], EVERYCULLING_SHUFFLEMASK(3, 3, 3, 3)); // r of twoPoint[0] , r of twoPoint[1], w of twoPoint[1] , w of twoPoint[1]
	posAB_rrrr = _mm_or_ps(culling::EVERYCULLING_M128F_ADD(posAB_rrrr, _mm_set1_ps(EVERYCULLING_BOUNDING_SPHRE_RADIUS_MARGIN)), _mm_set1_ps(-0.0f)); // rrrr of first twoPoint and rrrr of second twoPoint

	culling::EVERYCULLING_M128F dotPosAB45 = culling::EVERYCULLING_M128F_MUL_AND_ADD(posAB_zzzz, m128f_eightPlanes[6], m128f_eightPlanes[7]);
	dotPosAB45 = culling::EVERYCULLING_M128F_MUL_AND_ADD(posAB_yyyy, m128f_eightPlanes[5], dotPosAB45);
	dotPosAB45 = culling::EVERYCULLING_M128F_MUL_AND_ADD(posAB_xxxx, m128f_eightPlanes[4], dotPosAB45);

	dotPosA = _mm_cmpgt_ps(dotPosA, posA_rrrr); // if elemenet[i] have value 1, Pos A is in frustum Plane[i] ( 0 <= i < 4 )
	dotPosB = _mm_cmpgt_ps(dotPosB, posB_rrrr); // if elemenet[i] have value 1, Pos B is in frustum Plane[i] ( 0 <= i < 4 )
	dotPosAB45 = _mm_cmpgt_ps(dotPosAB45, posAB_rrrr);

	// this is wrong
	//dotPosA = _mm_cmpgt_ps(posA_rrrr, dotPosA); // if elemenet[i] have value 1, Pos A is in frustum Plane[i] ( 0 <= i < 4 )
	//dotPosB = _mm_cmpgt_ps(posB_rrrr, dotPosB); // if elemenet[i] have value 1, Pos B is in frustum Plane[i] ( 0 <= i < 4 )
	//dotPosAB45 = _mm_cmpgt_ps(posAB_rrrr, dotPosAB45);

	const culling::EVERYCULLING_M128F dotPosA45 = _mm_blend_ps(dotPosAB45, dotPosA, EVERYCULLING_SHUFFLEMASK(0, 3, 0, 0)); // Is In Plane with Plane[4], Plane[5], Plane[2], Plane[3]
	const culling::EVERYCULLING_M128F dotPosB45 = _mm_blend_ps(dotPosB, dotPosAB45, EVERYCULLING_SHUFFLEMASK(0, 3, 0, 0)); // Is In Plane with Plane[0], Plane[1], Plane[4], Plane[5]

	culling::EVERYCULLING_M128F RMaskA = _mm_and_ps(dotPosA, dotPosA45); //when everty bits is 1, PointA is in frustum
	culling::EVERYCULLING_M128F RMaskB = _mm_and_ps(dotPosB, dotPosB45);//when everty bits is 1, PointB is in frustum

	int IsPointAInFrustum = _mm_test_all_ones(*reinterpret_cast<EVERYCULLING_M128I*>(&RMaskA)); // value is 1, Point in in frustum
	int IsPointBInFrustum = _mm_test_all_ones(*reinterpret_cast<EVERYCULLING_M128I*>(&RMaskB));

	char IsPointABInFrustum = IsPointAInFrustum | (IsPointBInFrustum << 1);
	return IsPointABInFrustum;
}




void culling::ViewFrustumCulling::OnSetViewProjectionMatrix(const size_t cameraIndex, const culling::Mat4x4& cameraViewProjectionMatrix)
{
	culling::CullingModule::OnSetViewProjectionMatrix(cameraIndex, cameraViewProjectionMatrix);

	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);

	ExtractSIMDPlanesFromViewProjectionMatrix(cameraViewProjectionMatrix, mSIMDFrustumPlanes[cameraIndex].mFrustumPlanes, true);
}

void culling::ViewFrustumCulling::DebugOutputCullingResult(const size_t cameraIndex, culling::EntityBlock* const entityBlock) const
{
    for (size_t entityIndex = 0; entityIndex < entityBlock->mCurrentEntityCount; entityIndex++)
    {
        std::cout << "View Frustum Culling: Entity " << entityIndex 
                  << " culled: " << entityBlock->GetIsCulled(entityIndex, cameraIndex) 
                  << " Position: " << entityBlock->mWorldPositionAndWorldBoundingSphereRadius[entityIndex].GetPosition().x
                  << ", " << entityBlock->mWorldPositionAndWorldBoundingSphereRadius[entityIndex].GetPosition().y
                  << ", " << entityBlock->mWorldPositionAndWorldBoundingSphereRadius[entityIndex].GetPosition().z
                  << std::endl;
    }
}