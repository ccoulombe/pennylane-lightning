// Copyright 2018-2024 Xanadu Quantum Technologies Inc.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file
 * BLAS Lib dynamic loader manager.
 */
#pragma once

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

/// @cond DEV
#ifndef BLAS_LIB_PATH
#define BLAS_LIB_PATH ""
#endif
#ifndef BLAS_LIB_NAME
#define BLAS_LIB_NAME ""
#endif
/// @endcond

#include "SharedLibLoader.hpp"

namespace Pennylane::Util {
/**
 * @brief BLAS Lib dynamic loader manager.
 *
 * This class is a singleton that manages the dynamic loading of BLAS libraries.
 * It will search for the BLAS libraries using the path and name provided by CMake
 * via BLAS_LIB_PATH and BLAS_LIB_NAME macros, or fall back to system paths/RPATH.
 */
class BLASLibLoaderManager final {
  private:
    std::shared_ptr<SharedLibLoader> blasLib_;

    /**
     * @brief BLASLibLoaderManager.
     */
    explicit BLASLibLoaderManager() {
        std::string libPathStr;
        
        // Try to use the library path and name provided by CMake
        std::string blasLibPath(BLAS_LIB_PATH);
        std::string blasLibName(BLAS_LIB_NAME);
        
        if (!blasLibPath.empty() && !blasLibName.empty() && 
            std::filesystem::exists(blasLibPath)) {
            // Construct full path to library
            std::filesystem::path libPath(blasLibPath);
            
#ifdef __APPLE__
            std::string libFile = blasLibName + ".dylib";
#elif defined(_MSC_VER)
            std::string libFile = blasLibName + ".dll";
#else
            std::string libFile = blasLibName + ".so";
#endif
            auto fullPath = libPath / libFile;
            if (std::filesystem::exists(fullPath)) {
                libPathStr = fullPath.string();
            } else {
                // Try with lib prefix
                std::string libFileWithPrefix = "lib" + blasLibName.substr(blasLibName.find("lib") == 0 ? 3 : 0);
                fullPath = libPath / (libFileWithPrefix + 
#ifdef __APPLE__
                    ".dylib"
#elif defined(_MSC_VER)
                    ".dll"
#else
                    ".so"
#endif
                );
                if (std::filesystem::exists(fullPath)) {
                    libPathStr = fullPath.string();
                } else {
                    // Fall back to just the library name for RPATH search
                    libPathStr = blasLibName;
                }
            }
        } else {
            // Fall back to generic BLAS library name for system search
#ifdef __APPLE__
            libPathStr = "libblas.dylib";
#elif defined(_MSC_VER)
            libPathStr = "blas.dll";
#else
            libPathStr = "libblas.so";
#endif
        }

        blasLib_ = std::make_shared<SharedLibLoader>(libPathStr);
    }

  public:
    BLASLibLoaderManager(BLASLibLoaderManager &&) = delete;
    BLASLibLoaderManager(const BLASLibLoaderManager &) = delete;
    BLASLibLoaderManager &operator=(const BLASLibLoaderManager &) = delete;
    BLASLibLoaderManager operator=(const BLASLibLoaderManager &&) = delete;

    static auto getInstance() -> BLASLibLoaderManager & {
        static BLASLibLoaderManager instance;
        return instance;
    }

    ~BLASLibLoaderManager() = default;

    /**
     * @brief Get the BLAS library.
     *
     * @return SharedLibLoader* The BLAS library.
     */
    auto getBLASLib() -> SharedLibLoader * { return blasLib_.get(); }
};
} // namespace Pennylane::Util
