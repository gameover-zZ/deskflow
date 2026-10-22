/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "platform/OSXClipboard.h"

//! Clipboard converter that carries a single file published by Finder
/*!
Finder puts a public.file-url flavor on the pasteboard when the user copies a file.
toIClipboard() resolves that URL, reads the contents and packs them with
deskflow::packClipboardFile() so the file itself travels with the clipboard.

Only the receive direction is implemented: the macOS pasteboard writes every
flavor as a separate pasteboard item, so publishing files there would compete
with the text/image items the existing converters produce.
*/
class OSXClipboardFilesConverter : public IOSXClipboardConverter
{
public:
  OSXClipboardFilesConverter() = default;
  ~OSXClipboardFilesConverter() override = default;

  IClipboard::Format getFormat() const override;
  CFStringRef getOSXFormat() const override;

  std::string fromIClipboard(const std::string &) const override;
  std::string toIClipboard(const std::string &) const override;
};
