#include "EntityBlockViewer.h"

#include "Utils/CullingUtilities.h"
#include "Utils/MemoryUtils.h"

#include <cassert>
#include <iostream>
#include <algorithm>

bool culling::EntityBlockViewer::IsEntityBlockValid() const
{
    if (mTargetEntityBlock == nullptr)
    {
        std::cerr << "Error: mTargetEntityBlock is null" << std::endl;
        return false;
    }

    if (mEntityIndexInBlock >= mTargetEntityBlock->mCurrentEntityCount)
    {
        std::cerr << "Error: mEntityIndexInBlock is out of bounds" << std::endl;
        return false;
    }

    if (mTargetEntityBlock->mVertexDatas == nullptr)
    {
        std::cerr << "Error: mTargetEntityBlock->mVertexDatas is null" << std::endl;
        return false;
    }

    return true;
}

void culling::EntityBlockViewer::DeInitializeEntityBlockViewer()
{
	mTargetEntityBlock = nullptr;
	mEntityIndexInBlock = (std::uint64_t)-1;
}

void culling::EntityBlockViewer::ResetEntityData()
{
	SetIsObjectEnabled(true);
}

culling::EntityBlockViewer::EntityBlockViewer()
{
	DeInitializeEntityBlockViewer();
}

culling::EntityBlockViewer::EntityBlockViewer
(
	EntityBlock* const entityBlock, 
	const size_t entityIndexInBlock
)
	: mTargetEntityBlock{ entityBlock }, mEntityIndexInBlock{ entityIndexInBlock }
{
	assert(IsValid() == true);
	ResetEntityData();
}

culling::EntityBlockViewer::EntityBlockViewer(EntityBlockViewer&&) noexcept = default;
culling::EntityBlockViewer& culling::EntityBlockViewer::operator=(EntityBlockViewer&&) noexcept = default;

void culling::EntityBlockViewer::SetMeshVertexData
(
    const culling::Vec3* const vertices, 
    const std::uint64_t verticeCount,
    const std::uint32_t* const indices,
    const std::uint64_t indiceCount,
    const std::uint64_t verticeStride
)
{
    std::cout << "Entering SetMeshVertexData" << std::endl;
    std::cout << "this pointer: " << (void*)this << std::endl;
    std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;
    std::cout << "mEntityIndexInBlock: " << mEntityIndexInBlock << std::endl;

    if (this == nullptr) {
        std::cerr << "Error: 'this' pointer is null" << std::endl;
        return;
    }

    if (mTargetEntityBlock == nullptr) {
        std::cerr << "Error: mTargetEntityBlock is null" << std::endl;
        return;
    }

    std::cout << "Initial mTargetEntityBlock->mCurrentEntityCount: " << mTargetEntityBlock->mCurrentEntityCount << std::endl;

    if (IsValid() == true)
    {
        std::cout << "EntityBlockViewer is valid" << std::endl;
        std::cout << "Vertex pointer: " << (void*)vertices << std::endl;
        std::cout << "Index pointer: " << (void*)indices << std::endl;
        std::cout << "Vertex count: " << verticeCount << std::endl;
        std::cout << "Index count: " << indiceCount << std::endl;
        std::cout << "Vertex stride: " << verticeStride << std::endl;

        std::cout << "Before assigning data to mVertexDatas" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;
        std::cout << "mTargetEntityBlock->mCurrentEntityCount: " << mTargetEntityBlock->mCurrentEntityCount << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertices = vertices;
        std::cout << "After assigning mVertices" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVerticeCount = verticeCount;
        std::cout << "After assigning mVerticeCount" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndices = indices;
        std::cout << "After assigning mIndices" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndiceCount = indiceCount;
        std::cout << "After assigning mIndiceCount" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertexStride = verticeStride;
        std::cout << "After assigning mVertexStride" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;

        std::cout << "After assigning all data to mVertexDatas" << std::endl;
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;
        std::cout << "mTargetEntityBlock->mCurrentEntityCount: " << mTargetEntityBlock->mCurrentEntityCount << std::endl;

        // Calculate and store checksums
        std::cout << "Before calculating vertex checksum" << std::endl;
        uint64_t vertexChecksum = culling::calculateChecksum(vertices, verticeCount * sizeof(culling::Vec3));
        std::cout << "After calculating vertex checksum" << std::endl;

        std::cout << "Before calculating index checksum" << std::endl;
        uint64_t indexChecksum = culling::calculateChecksum(indices, indiceCount * sizeof(std::uint32_t));
        std::cout << "After calculating index checksum" << std::endl;

        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertexChecksum = vertexChecksum;
        mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndexChecksum = indexChecksum;

        std::cout << "Vertex checksum: " << vertexChecksum << std::endl;
        std::cout << "Index checksum: " << indexChecksum << std::endl;

        // Print first few vertices and indices
        const size_t maxVertsToPrint = 5;
        for (size_t i = 0; i < verticeCount && i < maxVertsToPrint; ++i) {
            std::cout << "Vertex " << i << ": " << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << std::endl;
        }

        const size_t maxIndicesToPrint = 10;
        for (size_t i = 0; i < indiceCount && i < maxIndicesToPrint; ++i) {
            std::cout << "Index " << i << ": " << indices[i] << std::endl;
        }

        std::cout << "SetMeshVertexData vertices address: " << (void*)vertices << std::endl;
        std::cout << "SetMeshVertexData indices address: " << (void*)indices << std::endl;

        // Protect the memory
        MemoryUtils::ProtectMemory((void*)vertices, verticeCount * sizeof(culling::Vec3));
        MemoryUtils::ProtectMemory((void*)indices, indiceCount * sizeof(std::uint32_t));
    }
    else
    {
        std::cerr << "Error: EntityBlockViewer is not valid in SetMeshVertexData" << std::endl;
    }

    std::cout << "Before final check" << std::endl;
    std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;
    if (mTargetEntityBlock != nullptr) {
        std::cout << "mTargetEntityBlock->mCurrentEntityCount: " << mTargetEntityBlock->mCurrentEntityCount << std::endl;
    } else {
        std::cerr << "Error: mTargetEntityBlock is null before final check" << std::endl;
    }

    std::cout << "Final check in SetMeshVertexData:" << std::endl;
    if (mTargetEntityBlock != nullptr) {
        std::cout << "mTargetEntityBlock pointer: " << (void*)mTargetEntityBlock << std::endl;
        std::cout << "mTargetEntityBlock->mCurrentEntityCount: " << mTargetEntityBlock->mCurrentEntityCount << std::endl;
        if (mTargetEntityBlock->mVertexDatas != nullptr) {
            std::cout << "mTargetEntityBlock->mVertexDatas pointer: " << (void*)mTargetEntityBlock->mVertexDatas << std::endl;
            if (mEntityIndexInBlock < mTargetEntityBlock->mCurrentEntityCount) {
                std::cout << "Stored vertices address: " << (void*)mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertices << std::endl;
                std::cout << "Stored vertex count: " << mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVerticeCount << std::endl;
                std::cout << "Stored indices address: " << (void*)mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndices << std::endl;
                std::cout << "Stored index count: " << mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndiceCount << std::endl;
                std::cout << "Stored vertex stride: " << mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertexStride << std::endl;
                std::cout << "Stored vertex checksum: " << mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mVertexChecksum << std::endl;
                std::cout << "Stored index checksum: " << mTargetEntityBlock->mVertexDatas[mEntityIndexInBlock].mIndexChecksum << std::endl;
            } else {
                std::cerr << "Error: mEntityIndexInBlock is out of bounds" << std::endl;
            }
        } else {
            std::cerr << "Error: mTargetEntityBlock->mVertexDatas is null" << std::endl;
        }
    } else {
        std::cerr << "Error: mTargetEntityBlock is null during final check" << std::endl;
    }

    std::cout << "Exiting SetMeshVertexData" << std::endl;
}