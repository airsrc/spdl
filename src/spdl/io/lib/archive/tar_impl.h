/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace spdl::archive::tar {
using TarMetaData = std::tuple<
    std::string, // filename
    uint64_t, // offset
    uint64_t // size
    >;

std::vector<TarMetaData> parse_tar_bytes(const char* root, const size_t len);
std::vector<TarMetaData> parse_tar_file(const std::string& path);

void load_bytes(
    const std::string& path,
    uint64_t offset,
    uint64_t size,
    char* dst);

std::string load_text(const std::string& path, uint64_t offset, uint64_t size);

} // namespace spdl::archive::tar
