/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "tar_impl.h"

#include <fmt/core.h>
#include <glog/logging.h>

#include <fstream>
#include <stdexcept>

namespace spdl::archive::tar {
namespace {
size_t parse_content_name_length(const char* p) {
  for (size_t i = 0; i < 100; ++i) {
    if ('\0' == p[i]) {
      return i;
    }
  }
  return 100;
}

void check_content_type(const char val) {
  switch (val) {
    case '0':
      [[fallthrough]];
    case '\0':
      return;
    case '1': // hard link
      break;
    case '2': // symbolic link
      break;
    case '3': // device file/special file
      break;
    case '4': // block device
      break;
    case '5': // directory
      break;
    case '6': // named pipe
      break;
  }
  throw std::domain_error("Only normal file is supported.");
}

size_t parse_content_size(const char* p) {
  size_t ret = 0;
  for (uint8_t i = 0; i < 11; ++i) {
    ret *= 8;
    if (auto v = p[i]; v > '0') {
      ret += v - '0';
    }
  }
  return ret;
}

} // namespace

std::vector<TarMetaData> parse_tar_bytes(
    const char* root,
    const size_t file_size) {
  std::vector<TarMetaData> ret;
  size_t offset = 0;
  while (offset + 512 < file_size) {
    check_content_type(root[offset + 156]);
    auto name_length = parse_content_name_length(root + offset);
    auto content_size = parse_content_size(root + offset + 124);
    if (offset + 512 + content_size > file_size) {
      throw std::domain_error(
          "Invalid metadata. The file extends to the outside of the given data.");
    }
    if (content_size != 0 && name_length != 0) {
      ret.emplace_back(TarMetaData{
          std::string_view{root + offset, name_length},
          offset + 512,
          content_size});
    }
    size_t increment = 512 + content_size;
    if (auto rem = increment % 512; rem) {
      increment += 512 - rem;
    }
    offset += increment;
  }
  return ret;
}

std::vector<TarMetaData> parse_tar_file(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error(fmt::format("Failed to open the file: {}", path));
  }

  file.seekg(0, std::ios::end);
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  char buffer[512];
  std::vector<TarMetaData> ret;
  size_t offset = 0;
  while (offset + 512 < file_size) {
    file.read(buffer, 512);
    auto name_length = parse_content_name_length(buffer);
    auto content_size = parse_content_size(buffer + 124);
    if (offset + 512 + content_size > file_size) {
      throw std::domain_error(
          "Invalid metadata. The file extends to the outside of the given data.");
    }
    if (content_size != 0 && name_length != 0) {
      ret.emplace_back(TarMetaData{
          std::string_view{buffer, name_length}, offset + 512, content_size});
    }
    size_t increment = content_size;
    if (auto rem = increment % 512; rem) {
      increment += 512 - rem;
    }
    file.seekg(increment, std::ios::cur);
    offset += 512 + increment;
  }
  return ret;
}

void load_bytes(
    const std::string& path,
    uint64_t offset,
    uint64_t size,
    char* dst) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error(fmt::format("Failed to open the file: {}", path));
  }

  if (offset > 0) {
    file.seekg(offset, std::ios::beg);
  }
  file.read(dst, size);
}

std::string load_text(const std::string& path, uint64_t offset, uint64_t size) {
  std::ifstream file(path, std::ios::in);
  if (!file.is_open()) {
    throw std::runtime_error(fmt::format("Failed to open the file: {}", path));
  }

  if (offset > 0) {
    file.seekg(offset, std::ios::beg);
  }
  std::string ret(size, '\0');
  file.read(ret.data(), size);
  return ret;
}
} // namespace spdl::archive::tar
