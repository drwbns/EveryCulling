# Add source files for DataType
set(DATA_TYPE_SOURCES
    Math/Common.cpp
    Math/SIMD_Core.cpp
    EntityBlock.cpp
    EntityBlockViewer.cpp
)

# Create a static library for DataType
add_library(DataType STATIC ${DATA_TYPE_SOURCES})