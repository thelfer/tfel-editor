/*!
 * \file   tests/TestUtilities.cxx
 * \brief  tests of the free functions of `TFEL/GUI/Utilities.hxx`.
 * \author Thomas Helfer
 * \date   04/11/2026
 */

#include <QtTest/QtTest>
#include <QtCore/QRegularExpression>
#include <QtCore/QTemporaryDir>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>

#include "TFEL/GUI/Utilities.hxx"

// the tests of the keyword under the cursor need a document but no window
// system: a `QTextDocument` and a `QTextCursor` are `QtGui`, not `QtWidgets`.
struct TestUtilities : QObject {
  Q_OBJECT

 private slots:
  void testFileNameRegExp();
  void testSuffixRegExp();
  void testFindFileInDirectoryOrParentDirectory();
  void testGetKeywordUnderCursor();
};

void TestUtilities::testFileNameRegExp() {
  const QRegularExpression re(tfel::gui::fileNameRegExp());
  QVERIFY(re.pattern() != QString());
  auto m = re.match("foo_1.hxx");
  QVERIFY(m.hasMatch());
  QCOMPARE(m.capturedLength(0), 9);
  // the pattern stops in front of anything a file name is not made of: it is
  // meant to be pasted in a larger pattern, as `suffixRegExp` does
  m = re.match("fo/o");
  QVERIFY(m.hasMatch());
  QCOMPARE(m.capturedLength(0), 2);
  m = re.match("foo bar.hxx");
  QVERIFY(m.hasMatch());
  QCOMPARE(m.capturedLength(0), 3);
}  // end of TestUtilities::testFileNameRegExp

void TestUtilities::testSuffixRegExp() {
  const auto re = tfel::gui::suffixRegExp("hxx");
  QVERIFY(re.match("foo.hxx").hasMatch());
  // the name part may hold everything `fileNameRegExp` accepts
  QVERIFY(re.match("a.b+c-d_1.hxx").hasMatch());
  // what the anchors are there for: a file of another suffix, or of the right
  // suffix without a name in front of it, is not a match
  QVERIFY(!re.match("foo.hpp").hasMatch());
  QVERIFY(!re.match("foo.hxx2").hasMatch());
  QVERIFY(!re.match("hxx").hasMatch());
  QVERIFY(!re.match("FOO.HXX").hasMatch());
}  // end of TestUtilities::testSuffixRegExp

void TestUtilities::testFindFileInDirectoryOrParentDirectory() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const auto root = dir.path();
  QVERIFY(QDir(root + "/a/b").mkpath("."));
  QFile needle(root + "/needle.txt");
  QVERIFY(needle.open(QIODevice::WriteOnly));
  needle.write("x");
  needle.close();
  // the file is in the directory itself
  QCOMPARE(tfel::gui::findFileInDirectoryOrParentDirectory(root, "needle.txt"),
           root);
  // and is found again walking up from a subdirectory, which is what the
  // search for a `compile_commands.json` or a project marker does
  QCOMPARE(tfel::gui::findFileInDirectoryOrParentDirectory(root + "/a/b",
                                                           "needle.txt"),
           root);
  // a file which is nowhere must be reported by the empty string: the first
  // iteration of the walk up used to hand back the parent directory whether or
  // not the file was there, which is a plausible-looking wrong answer
  QCOMPARE(tfel::gui::findFileInDirectoryOrParentDirectory(
               root + "/a/b", "no-such-file-1a2b3c.txt"),
           QString());
}  // end of TestUtilities::testFindFileInDirectoryOrParentDirectory

void TestUtilities::testGetKeywordUnderCursor() {
  auto keyword_at = [](const QString& s, const int p) {
    QTextDocument doc;
    doc.setPlainText(s);
    auto c = doc.find(s, 0);
    c.setPosition(p);
    return tfel::gui::getKeywordUnderCursor(c);
  };
  QCOMPARE(keyword_at("foo_bar = 42;", 2), QString("foo_bar"));
  QCOMPARE(keyword_at("foo_bar = 42;", 10), QString("42"));
  // a sign which holds no letter is no keyword, be it an assignment or the
  // semicolon which ends an MTest instruction
  QCOMPARE(keyword_at("foo_bar = 42;", 8), QString());
  QCOMPARE(keyword_at("foo_bar = 42;", 13), QString());
  // the '@' of an MTest instruction belongs to the keyword it introduces,
  // whether the cursor sits on the sign or in the word behind it
  QCOMPARE(keyword_at("@Materialproperty x", 1), QString("@Materialproperty"));
  QCOMPARE(keyword_at("@Materialproperty x", 5), QString("@Materialproperty"));
  // an '@' which introduces nothing is no keyword
  QCOMPARE(keyword_at("@ = 1", 0), QString());
}  // end of TestUtilities::testGetKeywordUnderCursor

QTEST_GUILESS_MAIN(TestUtilities)

#include "TestUtilities.moc"
