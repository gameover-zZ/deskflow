/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "ClipboardFilesTests.h"

#include "deskflow/ClipboardFiles.h"

namespace {

void writeUInt32(std::string &out, uint32_t value)
{
  out += static_cast<char>((value >> 24) & 0xff);
  out += static_cast<char>((value >> 16) & 0xff);
  out += static_cast<char>((value >> 8) & 0xff);
  out += static_cast<char>(value & 0xff);
}

//! Build a payload header without the matching body
std::string makeHeader(uint32_t nameSize, uint32_t dataSize)
{
  std::string payload;
  writeUInt32(payload, nameSize);
  payload += std::string(nameSize < 16 ? nameSize : 0, 'a');
  writeUInt32(payload, dataSize);
  return payload;
}

} // namespace

void ClipboardFilesTests::initTestCase()
{
  m_log.setFilter(LogLevel::Level::Verbose);
}

void ClipboardFilesTests::packAndUnpack()
{
  const deskflow::ClipboardFile input{"report.txt", "deskflow file clipboard"};

  deskflow::ClipboardFile output;
  QVERIFY(deskflow::unpackClipboardFile(deskflow::packClipboardFile(input), output));
  QCOMPARE(output.name, input.name);
  QCOMPARE(output.data, input.data);
}

void ClipboardFilesTests::packKeepsBinaryContent()
{
  std::string data;
  for (int i = 0; i < 256; ++i) {
    data += static_cast<char>(i);
  }

  const deskflow::ClipboardFile input{"binary.bin", data};

  deskflow::ClipboardFile output;
  QVERIFY(deskflow::unpackClipboardFile(deskflow::packClipboardFile(input), output));
  QCOMPARE(output.name, input.name);
  QCOMPARE(output.data.size(), data.size());
  QVERIFY(output.data == data);
}

void ClipboardFilesTests::unpackRejectsShortPayload()
{
  deskflow::ClipboardFile output;
  QVERIFY(!deskflow::unpackClipboardFile(std::string(3, '\0'), output));
}

void ClipboardFilesTests::unpackRejectsMissingName()
{
  deskflow::ClipboardFile output;
  QVERIFY(!deskflow::unpackClipboardFile(makeHeader(0, 0), output));
}

void ClipboardFilesTests::unpackRejectsTruncatedName()
{
  std::string payload;
  writeUInt32(payload, 100);
  writeUInt32(payload, 0);

  deskflow::ClipboardFile output;
  QVERIFY(!deskflow::unpackClipboardFile(payload, output));
}

void ClipboardFilesTests::unpackRejectsTruncatedData()
{
  std::string payload = makeHeader(5, 10);
  payload += "abc";

  deskflow::ClipboardFile output;
  QVERIFY(!deskflow::unpackClipboardFile(payload, output));
}

void ClipboardFilesTests::unpackRejectsFileBeyondLimit()
{
  const auto oversized = static_cast<uint32_t>(deskflow::kClipboardMaxFileSize + 1);
  std::string payload = makeHeader(5, oversized);

  deskflow::ClipboardFile output;
  QVERIFY(!deskflow::unpackClipboardFile(payload, output));
}

QTEST_MAIN(ClipboardFilesTests)
