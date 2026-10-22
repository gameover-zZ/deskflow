/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/MSWindowsClipboardFiles.h"

#include "base/Log.h"
#include "deskflow/ClipboardFiles.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QUuid>

#include <cstring>
#include <vector>

namespace {

//! Byte layout Windows expects behind a CF_HDROP handle
/*!
Equivalent to the DROPFILES structure, which lives in shellapi.h and is therefore
not part of this translation unit because WIN32_LEAN_AND_MEAN keeps that header
out of Windows.h.  The CF_HDROP layout is a stable part of the Win32 contract.
*/
struct DropFilesHeader
{
  DWORD pFiles;
  LONG ptX;
  LONG ptY;
  BOOL fNC;
  BOOL fWide;
};

static_assert(sizeof(DropFilesHeader) == 20, "CF_HDROP header must match DROPFILES");

QString stagingRoot()
{
  static const QString root =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/deskflow-clipboard");
  return root;
}

//! Strip characters that Windows forbids in a file name, the name comes from a peer
QString sanitizeFileName(const std::string &name)
{
  static const QRegularExpression invalidChars(QStringLiteral("[\\\\/:*?\"<>|]"));
  static const QRegularExpression controlChars(QStringLiteral("[\\x00-\\x1f]"));

  QString fileName = QString::fromStdString(name);
  fileName.remove(invalidChars);
  fileName.remove(controlChars);

  if (fileName.isEmpty() || fileName == QStringLiteral(".") || fileName == QStringLiteral("..")) {
    fileName = QStringLiteral("clipboard-file");
  }

  return fileName;
}

} // namespace

namespace deskflow {

HANDLE createWindowsFileDrop(const std::string &payload)
{
  ClipboardFile file;
  if (!unpackClipboardFile(payload, file)) {
    return nullptr;
  }

  // drop the previous staging area so files do not pile up in %TEMP%
  const QString root = stagingRoot();
  QDir(root).removeRecursively();

  const QString dir = root + QLatin1Char('/') + QUuid::createUuid().toString(QUuid::WithoutBraces);
  if (!QDir().mkpath(dir)) {
    LOG_WARN("failed to create clipboard staging directory: %s", dir.toUtf8().constData());
    return nullptr;
  }

  const QString path = dir + QLatin1Char('/') + sanitizeFileName(file.name);
  QFile output(path);
  if (!output.open(QIODevice::WriteOnly)) {
    LOG_WARN("failed to write clipboard file: %s", path.toUtf8().constData());
    return nullptr;
  }
  output.write(file.data.data(), static_cast<qint64>(file.data.size()));
  output.close();

  // CF_HDROP carries a double null terminated list of UTF-16 paths
  const auto nativePath = QDir::toNativeSeparators(path).toStdWString();
  std::vector<wchar_t> pathList(nativePath.begin(), nativePath.end());
  pathList.push_back(L'\0');
  pathList.push_back(L'\0');

  const auto pathListSize = pathList.size() * sizeof(wchar_t);
  const auto dropSize = sizeof(DropFilesHeader) + pathListSize;

  HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, dropSize);
  if (memory == nullptr) {
    LOG_WARN("failed to allocate clipboard file drop, size: %zu", dropSize);
    return nullptr;
  }

  auto *drop = static_cast<DropFilesHeader *>(GlobalLock(memory));
  if (drop == nullptr) {
    GlobalFree(memory);
    return nullptr;
  }

  drop->pFiles = static_cast<DWORD>(sizeof(DropFilesHeader));
  drop->fWide = TRUE;
  std::memcpy(reinterpret_cast<char *>(drop) + sizeof(DropFilesHeader), pathList.data(), pathListSize);

  GlobalUnlock(memory);

  LOG_DEBUG("staged clipboard file: %s, %zu bytes", path.toUtf8().constData(), file.data.size());

  return memory;
}

} // namespace deskflow
