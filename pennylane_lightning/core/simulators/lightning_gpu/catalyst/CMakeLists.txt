cmake_minimum_required(VERSION 3.20)

project(lightning_gpu_catalyst LANGUAGES CXX)

set(LGPU_CATALYST_FILES LightningGPUSimulator.cpp CACHE INTERNAL "")
add_library(lightning_gpu_catalyst SHARED ${LGPU_CATALYST_FILES})

include(FetchContent)

include("${pennylane_lightning_SOURCE_DIR}/cmake/support_catalyst.cmake")
FindCatalyst(lightning_gpu_catalyst)

target_include_directories(lightning_gpu_catalyst PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(lightning_gpu_catalyst PUBLIC  lightning_compile_options
                                                        lightning_gpu_algorithms
                                                        lightning_gpu_measurements
                                                        lightning_catalyst_core
)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
# Set BUILD_RPATH to include BLAS library path if ENABLE_BLAS is ON
if(BLAS_RUNTIME_LIB_PATH)
    set_target_properties(lightning_gpu_catalyst PROPERTIES BUILD_RPATH "${BLAS_RUNTIME_LIB_PATH}")
endif()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory("tests")
endif()
