/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <cstddef>
#include <string>

namespace deskflow {

//! Maximum size of a single file carried by the clipboard
/*!
Files larger than this are rejected before they are packed.  The whole payload is
kept in memory and pushed through a single marshalled clipboard transfer, so the
limit also keeps us well below the protocol message ceiling.
*/
inline constexpr size_t kClipboardMaxFileSize = 10 * 1024 * 1024;

//! A single file carried by the clipboard
struct ClipboardFile
{
  std::string name; //!< File name without directories, UTF-8
  std::string data; //!< Raw file contents
};

//! Pack a single file into the clipboard file payload
std::string packClipboardFile(const ClipboardFile &file);

//! Unpack a payload produced by packClipboardFile()
/*!
Returns false and leaves \p file untouched when the payload is truncated, malformed
or larger than kClipboardMaxFileSize.
*/
bool unpackClipboardFile(const std::string &payload, ClipboardFile &file);

} // namespace deskflow
