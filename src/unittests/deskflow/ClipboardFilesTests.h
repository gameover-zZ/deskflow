/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/Log.h"

#include <QTest>

class ClipboardFilesTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  // Test are run in order top to bottom
  void initTestCase();
  void packAndUnpack();
  void packKeepsBinaryContent();
  void unpackRejectsShortPayload();
  void unpackRejectsMissingName();
  void unpackRejectsTruncatedName();
  void unpackRejectsTruncatedData();
  void unpackRejectsFileBeyondLimit();

private:
  Log m_log;
};
