cmake_minimum_required(VERSION 3.20)

set(LOGO [=[
░█░░░▀█▀░█▀▀░█░█░▀█▀░█▀█░▀█▀░█▀█░█▀▀░░░░▄▀▄░█░█░█▀▄░▀█▀░▀█▀
░█░░░░█░░█░█░█▀█░░█░░█░█░░█░░█░█░█░█░░░░█\█░█░█░█▀▄░░█░░░█░
░▀▀▀░▀▀▀░▀▀▀░▀░▀░░▀░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀░░░▀\░▀▀▀░▀▀░░▀▀▀░░▀░
]=])
message(${LOGO})

project(lightning_qubit
        DESCRIPTION "PennyLane Lightning C++ Backend."
        LANGUAGES CXX C
)

set(LQUBIT_FILES    StateVectorLQubitManaged.cpp
                    StateVectorLQubitRaw.cpp
                    CACHE INTERNAL "" FORCE)

add_library(lightning_qubit STATIC ${LQUBIT_FILES})

option(ENABLE_BLAS "Enable BLAS" OFF)
option(ENABLE_GATE_DISPATCHER "Enable gate kernel dispatching on AVX/AVX2/AVX512" ON)
option(LQ_ENABLE_KERNEL_OMP "Enable OpenMP pragmas for gate kernels" OFF)
option(LQ_ENABLE_KERNEL_AVX_STREAMING "Enable AVX2/512 streaming operations for gate kernels" OFF)

# Inform the compiler that this device is enabled.
target_compile_options(lightning_compile_options INTERFACE "-D_ENABLE_PLQUBIT=1")

if(ENABLE_BLAS)
    message(STATUS "ENABLE_BLAS is ON.")

    # Use standard CMake FindBLAS and FindLAPACK modules
    # Supports BLA_VENDOR (e.g., OpenBLAS, FlexiBLAS, Intel10_64lp, ATLAS, etc.)
    find_package(BLAS REQUIRED)
    find_package(LAPACK REQUIRED)

    if(BLAS_FOUND)
        message(STATUS "BLAS libraries: ${BLAS_LIBRARIES}")
        # Extract library paths from BLAS_LIBRARIES for runtime loading
        foreach(lib ${BLAS_LIBRARIES})
            if(EXISTS "${lib}")
                get_filename_component(lib_path "${lib}" DIRECTORY)
                list(APPEND BLAS_LIB_PATHS "${lib_path}")
                get_filename_component(lib_name "${lib}" NAME_WE)
                list(APPEND BLAS_LIB_NAMES "${lib_name}")
            endif()
        endforeach()
        if(BLAS_LIB_PATHS)
            list(REMOVE_DUPLICATES BLAS_LIB_PATHS)
            list(GET BLAS_LIB_PATHS 0 BLAS_LIB_PATH)
            message(STATUS "BLAS library path: ${BLAS_LIB_PATH}")
            # Set as cache variable so it's available to catalyst subdirectories
            set(BLAS_RUNTIME_LIB_PATH "${BLAS_LIB_PATH}" CACHE INTERNAL "BLAS library runtime path")
        endif()
        if(BLAS_LIB_NAMES)
            list(GET BLAS_LIB_NAMES 0 BLAS_LIB_NAME)
            message(STATUS "BLAS library name: ${BLAS_LIB_NAME}")
            # Pass the library path and name to C++ code
            add_compile_definitions(BLAS_LIB_PATH="${BLAS_LIB_PATH}")
            add_compile_definitions(BLAS_LIB_NAME="${BLAS_LIB_NAME}")
        endif()
    endif()

    if(LAPACK_FOUND)
        message(STATUS "LAPACK libraries: ${LAPACK_LIBRARIES}")
    endif()

    target_link_libraries(lightning_external_libs INTERFACE ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
    target_compile_options(lightning_compile_options INTERFACE "-D_ENABLE_BLAS=1")
else()
    message(STATUS "ENABLE_BLAS is OFF.")
    # Set empty path when BLAS is disabled
    set(BLAS_RUNTIME_LIB_PATH "" CACHE INTERNAL "BLAS library runtime path")
endif()

if(LQ_ENABLE_KERNEL_OMP)
    message(STATUS "OpenMP-parallelized kernels: ON.")
    add_definitions("-DPL_LQ_KERNEL_OMP")
    target_compile_definitions(lightning_qubit PUBLIC -DPL_LQ_KERNEL_OMP)
else()
    message(STATUS "OpenMP-parallelized kernels: OFF.")
endif()

if(LQ_ENABLE_KERNEL_AVX_STREAMING)
    if(NOT LQ_ENABLE_KERNEL_OMP)
        message(WARNING "AVX streaming operations require `LQ_ENABLE_KERNEL_OMP` to be enabled.")
    endif()
    message(STATUS "AVX streaming operations: ON.")
    add_definitions("-DPL_LQ_KERNEL_AVX_STREAMING")
else()
    message(STATUS "AVX streaming operations: OFF.")
endif()

target_link_libraries(lightning_qubit PUBLIC    lightning_compile_options
                                                lightning_external_libs
                                                lightning_base
                                                lightning_utils
                                                lightning_qubit_utils
                                                lightning_gates
                                                lightning_qubit_gates
                                                )

target_include_directories(lightning_qubit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET lightning_qubit PROPERTY POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Include subdirectories
###############################################################################
set(COMPONENT_SUBDIRS   algorithms
                        bindings
                        gates
                        measurements
                        observables
                        utils
                        )

# Do not build the LQ plugin for Windows
if (NOT WIN32)
    set(COMPONENT_SUBDIRS   ${COMPONENT_SUBDIRS}
                            catalyst
                            )
endif()

foreach(COMP ${COMPONENT_SUBDIRS})
    add_subdirectory(${COMP})
endforeach()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory("tests")
endif()
