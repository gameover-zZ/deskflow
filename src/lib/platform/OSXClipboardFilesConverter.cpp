/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/OSXClipboardFilesConverter.h"

#include "base/Log.h"
#include "deskflow/ClipboardFiles.h"

#include <QFile>
#include <QFileInfo>
#include <QString>

namespace {

//! Convert the raw public.file-url flavor bytes into a local filesystem path
QString urlDataToPath(const std::string &urlData)
{
  CFURLRef url = CFURLCreateWithBytes(
      kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(urlData.data()), static_cast<CFIndex>(urlData.size()),
      kCFStringEncodingUTF8, nullptr
  );
  if (url == nullptr) {
    return {};
  }

  CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
  CFRelease(url);
  if (path == nullptr) {
    return {};
  }

  const QString result = QString::fromCFString(path);
  CFRelease(path);

  return result;
}

} // namespace

IClipboard::Format OSXClipboardFilesConverter::getFormat() const
{
  return IClipboard::Format::Files;
}

CFStringRef OSXClipboardFilesConverter::getOSXFormat() const
{
  return CFSTR("public.file-url");
}

std::string OSXClipboardFilesConverter::fromIClipboard(const std::string &) const
{
  return std::string();
}

std::string OSXClipboardFilesConverter::toIClipboard(const std::string &urlData) const
{
  const auto path = urlDataToPath(urlData);
  if (path.isEmpty()) {
    LOG_DEBUG("rejecting clipboard file url, cannot be parsed");
    return std::string();
  }

  const QFileInfo info(path);
  if (!info.isFile()) {
    LOG_DEBUG("rejecting clipboard file url, not a regular file: %s", path.toUtf8().constData());
    return std::string();
  }

  if (static_cast<size_t>(info.size()) > deskflow::kClipboardMaxFileSize) {
    LOG_WARN("rejecting clipboard file, exceeds limit: %lld bytes", static_cast<long long>(info.size()));
    return std::string();
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_WARN("failed to open clipboard file: %s", path.toUtf8().constData());
    return std::string();
  }

  deskflow::ClipboardFile clipboardFile;
  clipboardFile.name = info.fileName().toStdString();
  clipboardFile.data = file.readAll().toStdString();

  LOG_DEBUG("packing clipboard file: %s, %zu bytes", clipboardFile.name.c_str(), clipboardFile.data.size());

  return deskflow::packClipboardFile(clipboardFile);
}
