/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "deskflow/ClipboardFiles.h"

#include "base/Log.h"

#include <cstdint>
#include <utility>

namespace {

//! name length + data length
constexpr size_t kFixedHeaderSize = 8;

//! Guard against a peer advertising an absurd file name
constexpr uint32_t kMaxNameSize = 1024;

void writeUInt32(std::string &out, uint32_t value)
{
  out += static_cast<char>((value >> 24) & 0xff);
  out += static_cast<char>((value >> 16) & 0xff);
  out += static_cast<char>((value >> 8) & 0xff);
  out += static_cast<char>(value & 0xff);
}

uint32_t readUInt32(const char *data)
{
  const auto *ubuf = reinterpret_cast<const unsigned char *>(data);
  return (static_cast<uint32_t>(ubuf[0]) << 24) | (static_cast<uint32_t>(ubuf[1]) << 16) |
         (static_cast<uint32_t>(ubuf[2]) << 8) | static_cast<uint32_t>(ubuf[3]);
}

} // namespace

namespace deskflow {

std::string packClipboardFile(const ClipboardFile &file)
{
  std::string payload;
  payload.reserve(kFixedHeaderSize + file.name.size() + file.data.size());

  writeUInt32(payload, static_cast<uint32_t>(file.name.size()));
  payload += file.name;
  writeUInt32(payload, static_cast<uint32_t>(file.data.size()));
  payload += file.data;

  return payload;
}

bool unpackClipboardFile(const std::string &payload, ClipboardFile &file)
{
  if (payload.size() < kFixedHeaderSize) {
    LOG_WARN("rejecting clipboard file payload, too short: %zu bytes", payload.size());
    return false;
  }

  const char *cursor = payload.data();
  const char *const end = cursor + payload.size();

  const uint32_t nameSize = readUInt32(cursor);
  cursor += 4;

  if (nameSize == 0 || nameSize > kMaxNameSize) {
    LOG_WARN("rejecting clipboard file payload, invalid name size: %u", nameSize);
    return false;
  }

  // the data length field must still be readable after the name
  if (static_cast<size_t>(end - cursor) < static_cast<size_t>(nameSize) + 4) {
    LOG_WARN("rejecting clipboard file payload, truncated name: %u bytes", nameSize);
    return false;
  }

  std::string name(cursor, nameSize);
  cursor += nameSize;

  const uint32_t dataSize = readUInt32(cursor);
  cursor += 4;

  if (dataSize > kClipboardMaxFileSize) {
    LOG_WARN("rejecting clipboard file payload, file exceeds limit: %u bytes", dataSize);
    return false;
  }

  if (static_cast<size_t>(end - cursor) < dataSize) {
    LOG_WARN("rejecting clipboard file payload, truncated data: %u bytes", dataSize);
    return false;
  }

  file.name = std::move(name);
  file.data.assign(cursor, dataSize);

  return true;
}

} // namespace deskflow
