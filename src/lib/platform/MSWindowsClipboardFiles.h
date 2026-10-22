/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

namespace deskflow {

//! Build a CF_HDROP handle for the file carried by \p payload
/*!
The file is written to a staging directory under %TEMP% first because CF_HDROP
only references paths that exist on disk; Explorer reads the file when the user
pastes.  The staging directory is replaced on every call.

Returns nullptr when the payload is malformed, exceeds kClipboardMaxFileSize or
the file cannot be written.  The caller owns the returned handle.
*/
HANDLE createWindowsFileDrop(const std::string &payload);

} // namespace deskflow
