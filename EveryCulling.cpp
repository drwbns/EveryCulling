#include "EveryCulling.h"

#include <thread>
#include <iostream>

#include "DataType/EntityBlock.h"
#include "CullingModule/ViewFrustumCulling/ViewFrustumCulling.h"
#include "CullingModule/PreCulling/PreCulling.h"
#include "CullingModule/DistanceCulling/DistanceCulling.h"
#include "CullingModule/MaskedSWOcclusionCulling/MaskedSWOcclusionCulling.h"


void culling::EveryCulling::FreeEntityBlock(EntityBlock* freedEntityBlock)
{
	assert(freedEntityBlock != nullptr);

	size_t freedEntityBlockIndex;
	const size_t entityBlockCount = mActiveEntityBlockList.size();
	bool IsSuccessToFind = false;
	for (size_t i = 0; i < entityBlockCount; i++)
	{
		//Freeing entity block happen barely
		//So this looping is acceptable
		if (mActiveEntityBlockList[i] == freedEntityBlock)
		{
			freedEntityBlockIndex = i;
			IsSuccessToFind = true;
			break;
		}
	}

	assert(IsSuccessToFind == true);

	freedEntityBlock->bIsValidEntityBlock = false;
	freedEntityBlock->mEntityBlockUniqueID = EVERYCULLING_INVALID_ENTITY_UNIQUE_ID_MAGIC_NUMBER;
	mFreeEntityBlockList.push_back(freedEntityBlock);

	if(IsSuccessToFind == true)
	{
		mActiveEntityBlockList.erase(mActiveEntityBlockList.begin() + freedEntityBlockIndex);
	}
	
}


culling::EntityBlock* culling::EveryCulling::GetNewEntityBlockFromPool()
{
	if (mFreeEntityBlockList.size() == 0)
	{
		AllocateEntityBlockPool();
	}

	assert(mFreeEntityBlockList.size() != 0);
	EntityBlock* entityBlock = mFreeEntityBlockList.back();

	entityBlock->bIsValidEntityBlock = true;
	entityBlock->mEntityBlockUniqueID = mEntityBlockUniqueIDCounter;
	++mEntityBlockUniqueIDCounter;

	mFreeEntityBlockList.pop_back();
	return entityBlock;
}

void culling::EveryCulling::ResetCullingModules()
{
	for (auto cullingModule : mUpdatedCullingModules)
	{
		cullingModule->ResetCullingModule(mCurrentTickCount);
	}
	
	mMaskedSWOcclusionCulling->ResetState(mCurrentTickCount);

}

void culling::EveryCulling::ResetEntityBlocks()
{
	//Maybe Compiler use SIMD or do faster than SIMD instruction
	for (auto entityBlock : mActiveEntityBlockList)
	{
		entityBlock->ResetEntityBlock(mCurrentTickCount);
	}
}

void culling::EveryCulling::AllocateEntityBlockPool()
{
	EntityBlock* newEntityBlockChunk = new EntityBlock[EVERYCULLING_INITIAL_ENTITY_BLOCK_COUNT];
	for (std::uint32_t i = 0; i < EVERYCULLING_INITIAL_ENTITY_BLOCK_COUNT; i++)
	{
		newEntityBlockChunk[i].bIsValidEntityBlock = false;
		newEntityBlockChunk[i].mEntityBlockUniqueID = EVERYCULLING_INVALID_ENTITY_UNIQUE_ID_MAGIC_NUMBER;

		mFreeEntityBlockList.push_back(newEntityBlockChunk + i);
	}
	mAllocatedEntityBlockChunkList.push_back(newEntityBlockChunk);
}


void culling::EveryCulling::RemoveEntityFromBlock(EntityBlock* ownerEntityBlock, std::uint32_t entityIndexInBlock)
{
	assert(ownerEntityBlock != nullptr);
	assert(entityIndexInBlock >= 0 && entityIndexInBlock < EVERYCULLING_ENTITY_COUNT_IN_ENTITY_BLOCK);

	for(auto cullingModule : mUpdatedCullingModules)
	{
		cullingModule->ClearEntityData(ownerEntityBlock, entityIndexInBlock);
	}
	
	assert(ownerEntityBlock->mCurrentEntityCount != 0);
	ownerEntityBlock->mCurrentEntityCount--;
	if (ownerEntityBlock->mCurrentEntityCount == 0)
	{
		FreeEntityBlock(ownerEntityBlock);
	}
	
}

void culling::EveryCulling::ThreadCullJob(const size_t cameraIndex, const unsigned long long tickCount)
{
    std::cout << "Starting culling job for camera " << cameraIndex << std::endl;

	const std::uint32_t entityBlockCount = static_cast<std::uint32_t>(GetActiveEntityBlockCount());
	const unsigned long long currentTickCount = mCurrentTickCount;

	if (entityBlockCount > 0 && currentTickCount == tickCount)
	{
		mRunningThreadCount++;

		for (size_t moduleIndex = 0; moduleIndex < mUpdatedCullingModules.size(); moduleIndex++)
		{
			culling::CullingModule* cullingModule = mUpdatedCullingModules[moduleIndex];
			assert(cullingModule != nullptr);

			if (cullingModule->IsEnabled == true)
			{
				OnStartCullingModule(cullingModule);

				cullingModule->ThreadCullJob(cameraIndex, currentTickCount);

				std::atomic_thread_fence(std::memory_order_seq_cst);

				while (cullingModule->GetFinishedThreadCount(cameraIndex) < mRunningThreadCount)
				{
					
				}

				OnEndCullingModule(cullingModule);
			}
		}
	}
}


void culling::EveryCulling::WaitToFinishCullJob(const std::uint32_t cameraIndex) const
{
	const CullingModule* lastEnabledCullingModule = GetLastEnabledCullingModule();
	if(lastEnabledCullingModule != nullptr)
	{
		while (lastEnabledCullingModule->GetFinishedThreadCount(cameraIndex) < mRunningThreadCount)
		{

		}
	}
}

void culling::EveryCulling::WaitToFinishCullJobOfAllCameras() const
{
	for (std::uint32_t cameraIndex = 0; cameraIndex < mCameraCount; cameraIndex++)
	{
		WaitToFinishCullJob(cameraIndex);
	}
}

void culling::EveryCulling::PreCullJob()
{
	mCurrentTickCount++;
	mRunningThreadCount = 0;

	ResetEntityBlocks();
	ResetCullingModules();

	//release!
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

const culling::CullingModule* culling::EveryCulling::GetLastEnabledCullingModule() const
{
	culling::CullingModule* lastEnabledCullingModule = nullptr;
	for(int cullingModuleIndex = static_cast<int>(mUpdatedCullingModules.size()) - 1 ; cullingModuleIndex >= 0 ; cullingModuleIndex--)
	{
		if(mUpdatedCullingModules[cullingModuleIndex]->IsEnabled == true)
		{
			lastEnabledCullingModule = mUpdatedCullingModules[cullingModuleIndex];
			break;
		}
	}
	return lastEnabledCullingModule;
}

void culling::EveryCulling::SetEnabledCullingModule(const CullingModuleType cullingModuleType, const bool isEnabled)
{
	switch (cullingModuleType)
	{

	case CullingModuleType::PreCulling:
		mPreCulling->IsEnabled = isEnabled;
		break;

	case CullingModuleType::ViewFrustumCulling:
		mViewFrustumCulling->IsEnabled = isEnabled;
		break;

	case CullingModuleType::MaskedSWOcclusionCulling:

		mMaskedSWOcclusionCulling->mSolveMeshRoleStage.IsEnabled = isEnabled;
		mMaskedSWOcclusionCulling->mBinTrianglesStage.IsEnabled = isEnabled;
		mMaskedSWOcclusionCulling->mRasterizeTrianglesStage.IsEnabled = isEnabled;
		mMaskedSWOcclusionCulling->mQueryOccludeeStage.IsEnabled = isEnabled;
		break;

	case CullingModuleType::DistanceCulling:

		mDistanceCulling->IsEnabled = isEnabled;
		break;
		
	}
}

std::uint32_t culling::EveryCulling::GetRunningThreadCount() const
{
	return mRunningThreadCount;
}

culling::EntityBlock* culling::EveryCulling::AllocateNewEntityBlockFromPool()
{
	std::cout << "Entering AllocateNewEntityBlockFromPool" << std::endl;

	EntityBlock* const newEntityBlock = GetNewEntityBlockFromPool();
	newEntityBlock->ClearEntityBlock();

	mActiveEntityBlockList.push_back(newEntityBlock);

	std::cout << "New EntityBlock allocated:" << std::endl;
	std::cout << "EntityBlock pointer: " << (void*)newEntityBlock << std::endl;
	std::cout << "Active entity block count: " << mActiveEntityBlockList.size() << std::endl;

	std::cout << "Exiting AllocateNewEntityBlockFromPool" << std::endl;
	return newEntityBlock;
}





culling::EntityBlockViewer culling::EveryCulling::AllocateNewEntity()
{
	std::cout << "Entering AllocateNewEntity" << std::endl;

	culling::EntityBlock* targetEntityBlock;
	if (mActiveEntityBlockList.size() == 0)
	{
		std::cout << "No active entity blocks, allocating new block" << std::endl;
		// if Any entityBlock isn't allocated yet
		targetEntityBlock = AllocateNewEntityBlockFromPool();
	}
	else
	{//When Allocated entity block count is at least one
		std::cout << "Active entity blocks exist" << std::endl;
		//Get last entityblock in active entities
		targetEntityBlock = { mActiveEntityBlockList.back() };

		if (targetEntityBlock->mCurrentEntityCount == EVERYCULLING_ENTITY_COUNT_IN_ENTITY_BLOCK)
		{
			std::cout << "Last entity block is full, allocating new block" << std::endl;
			//if last entityblock in active entities is full of entities
			//alocate new entity block
			targetEntityBlock = AllocateNewEntityBlockFromPool();
		}
	}

	assert(targetEntityBlock->mCurrentEntityCount <= EVERYCULLING_ENTITY_COUNT_IN_ENTITY_BLOCK); // something is weird........
	
	targetEntityBlock->mCurrentEntityCount++;
	
	
	EntityBlockViewer newViewer(targetEntityBlock, targetEntityBlock->mCurrentEntityCount - 1);
	
	std::cout << "New entity allocated:" << std::endl;
	std::cout << "EntityBlock pointer: " << (void*)targetEntityBlock << std::endl;
	std::cout << "Entity index: " << (targetEntityBlock->mCurrentEntityCount - 1) << std::endl;
	std::cout << "Current entity count: " << targetEntityBlock->mCurrentEntityCount << std::endl;

	std::cout << "New EntityBlockViewer:" << std::endl;
    std::cout << "EntityBlockViewer this pointer: " << (void*)&newViewer << std::endl;
    std::cout << "EntityBlockViewer mTargetEntityBlock: " << (void*)newViewer.mTargetEntityBlock << std::endl;
    std::cout << "EntityBlockViewer mEntityIndexInBlock: " << newViewer.mEntityIndexInBlock << std::endl;

    std::cout << "Exiting AllocateNewEntity" << std::endl;
	return newViewer;
}

void culling::EveryCulling::RemoveEntityFromBlock(EntityBlockViewer& entityBlockViewer)
{
	if(entityBlockViewer.IsValid() == true)
	{
		RemoveEntityFromBlock(entityBlockViewer.mTargetEntityBlock, entityBlockViewer.mEntityIndexInBlock);
		entityBlockViewer.DeInitializeEntityBlockViewer();
	}

	//Don't decrement mEntityGridCell.AllocatedEntityCountInBlocks
	//Entities Indexs in EntityBlock should not be swapped because already allocated EntityBlockViewer can't see it
}

culling::EveryCulling::EveryCulling(const std::uint32_t resolutionWidth, const std::uint32_t resolutionHeight)
	:
	mPreCulling{ std::make_unique<PreCulling>(this) },
	mDistanceCulling{ std::make_unique<DistanceCulling>(this) },
	mViewFrustumCulling{ std::make_unique<ViewFrustumCulling>(this) }
#ifdef ENABLE_SCREEN_SAPCE_AABB_CULLING
	, mScreenSpaceBoudingSphereCulling{ std::make_unique<ScreenSpaceBoundingSphereCulling>(this) }
#endif
	, mMaskedSWOcclusionCulling{ std::make_unique<MaskedSWOcclusionCulling>(this, resolutionWidth, resolutionHeight) }
	, mUpdatedCullingModules
		{
			mPreCulling.get(),
			mDistanceCulling.get(),
			mViewFrustumCulling.get(),
			mMaskedSWOcclusionCulling.get(), // Choose Role Stage
			&(mMaskedSWOcclusionCulling->mSolveMeshRoleStage), // Choose Role Stage
			&(mMaskedSWOcclusionCulling->mBinTrianglesStage), // BinTriangles
			&(mMaskedSWOcclusionCulling->mRasterizeTrianglesStage), // DrawOccluderStage
			&(mMaskedSWOcclusionCulling->mQueryOccludeeStage) // QueryOccludeeStage
		}
#ifdef EVERYCULLING_PROFILING_CULLING
	, mEveryCullingProfiler{}
#endif
	, mCurrentTickCount()
	, bmIsEntityBlockPoolInitialized(false)
	, mEntityBlockUniqueIDCounter{0}
{
	//to protect 
	mFreeEntityBlockList.reserve(EVERYCULLING_INITIAL_ENTITY_BLOCK_RESERVED_SIZE);
	mActiveEntityBlockList.reserve(EVERYCULLING_INITIAL_ENTITY_BLOCK_RESERVED_SIZE);

	AllocateEntityBlockPool();

	//CacheCullBlockEntityJobs();
	bmIsEntityBlockPoolInitialized = true;
}

culling::EveryCulling::~EveryCulling()
{
	for (culling::EntityBlock* allocatedEntityBlockChunk : mAllocatedEntityBlockChunkList)
	{
		delete[] allocatedEntityBlockChunk;
	}
}

void culling::EveryCulling::SetCameraCount(const size_t cameraCount)
{
	mCameraCount = cameraCount;

	for (auto updatedCullingModule : mUpdatedCullingModules)
	{
		updatedCullingModule->OnSetCameraCount(cameraCount);
	}
}

unsigned long long culling::EveryCulling::GetTickCount() const
{
	return mCurrentTickCount;
}

void culling::EveryCulling::OnStartCullingModule(const culling::CullingModule* const cullingModule)
{
#ifdef EVERYCULLING_PROFILING_CULLING
	mEveryCullingProfiler.SetStartTime(cullingModule->GetCullingModuleName());
#endif
}

void culling::EveryCulling::OnEndCullingModule(const culling::CullingModule* const cullingModule)
{
#ifdef EVERYCULLING_PROFILING_CULLING
	mEveryCullingProfiler.SetEndTime(cullingModule->GetCullingModuleName());
#endif
}

void culling::EveryCulling::SetViewProjectionMatrix(const size_t cameraIndex, const culling::Mat4x4& viewProjectionMatrix)
{
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);

	EVERYCULLING_ALIGNMENT_ASSERT(reinterpret_cast<size_t>(&viewProjectionMatrix), 32);

	mCameraViewProjectionMatrixes[cameraIndex] = viewProjectionMatrix;
	
	if (cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT)
	{
		for (auto updatedCullingModule : mUpdatedCullingModules)
		{
			updatedCullingModule->OnSetViewProjectionMatrix(cameraIndex, viewProjectionMatrix);
		}
	}
}

void culling::EveryCulling::SetFieldOfViewInDegree(const size_t cameraIndex, const float fov)
{
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);
	assert(fov > 0.0f);

	mCameraFieldOfView[cameraIndex] = fov;

	if (cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT)
	{
		for (auto updatedCullingModule : mUpdatedCullingModules)
		{
			updatedCullingModule->OnSetCameraFieldOfView(cameraIndex, fov);
		}
	}
}

void culling::EveryCulling::SetCameraNearFarClipPlaneDistance
(
	const size_t cameraIndex,
	const float nearPlaneDistance, 
	const float farPlaneDistance
)
{
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);
	assert(nearPlaneDistance > 0.0f);
	assert(farPlaneDistance > 0.0f);
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);


	mNearClipPlaneDistance[cameraIndex] = nearPlaneDistance;
	mFarClipPlaneDistance[cameraIndex] = farPlaneDistance;

	if (cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT)
	{
		for (auto updatedCullingModule : mUpdatedCullingModules)
		{
			updatedCullingModule->OnSetCameraNearClipPlaneDistance(cameraIndex, nearPlaneDistance);
			updatedCullingModule->OnSetCameraFarClipPlaneDistance(cameraIndex, farPlaneDistance);
		}
	}
}

void culling::EveryCulling::SetCameraWorldPosition(const size_t cameraIndex, const culling::Vec3& cameraWorldPos)
{
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);

	mCameraWorldPositions[cameraIndex] = cameraWorldPos;

	if (cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT)
	{
		for (auto updatedCullingModule : mUpdatedCullingModules)
		{
			updatedCullingModule->OnSetCameraWorldPosition(cameraIndex, cameraWorldPos);
		}
	}
}

void culling::EveryCulling::SetCameraRotation(const size_t cameraIndex, const culling::Vec4& cameraRotation)
{
	assert(cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT);

	mCameraRotations[cameraIndex] = cameraRotation;

	if (cameraIndex >= 0 && cameraIndex < EVERYCULLING_MAX_CAMERA_COUNT)
	{
		for (auto updatedCullingModule : mUpdatedCullingModules)
		{
			updatedCullingModule->OnSetCameraRotation(cameraIndex, cameraRotation);
		}
	}
}

void culling::EveryCulling::UpdateGlobalDataForCullJob(const size_t cameraIndex, const GlobalDataForCullJob& settingParameters)
{
	SetViewProjectionMatrix(cameraIndex, settingParameters.mViewProjectionMatrix);
	SetFieldOfViewInDegree(cameraIndex, settingParameters.mFieldOfViewInDegree);
	SetCameraNearFarClipPlaneDistance(cameraIndex, settingParameters.mCameraNearPlaneDistance, settingParameters.mCameraFarPlaneDistance);
	SetCameraWorldPosition(cameraIndex, settingParameters.mCameraWorldPosition);
	SetCameraRotation(cameraIndex, settingParameters.mCameraRotation);
}

const std::vector<culling::EntityBlock*>& culling::EveryCulling::GetActiveEntityBlockList() const
{
	return mActiveEntityBlockList;
}

size_t culling::EveryCulling::GetActiveEntityBlockCount() const
{
	return GetActiveEntityBlockList().size();
}
