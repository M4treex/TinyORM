#include <QCoreApplication>
#include <QtTest>

#include <qt_windows.h>

#include "fs.hpp"

// TinyORM and TinyUtils version files
#include "versiondebug_qmake.hpp"

using namespace TestUtils;

class tst_Version : public QObject
{
    Q_OBJECT

private slots:
    void versions_TinyOrm() const;
    void versions_TinyUtils() const;

    void checkFileVersion_TinyOrm() const;
    void checkFileVersion_TinyUtils() const;

private:
    /*! Return value for the getExeVersionString(). */
    struct FileVersions
    {
        /*! ProductVersion. */
        const QString productVersion = "";
        /*! FileVersion. */
        const QString fileVersion = "";
    };

    /*! Obtain a ProductVersion and FileVersion strings from an exe/dll. */
    FileVersions getExeVersionString(const QString &fileName) const;
};

void tst_Version::versions_TinyOrm() const
{
    // Test types
    QCOMPARE(typeid (TINYORM_VERSION_MAJOR), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_MINOR), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_BUGFIX), typeid (int));
    QCOMPARE(typeid (TINYORM_VERSION_BUILD), typeid (int));

    // Individual version numbers have to be greater than zero
    QVERIFY(TINYORM_VERSION_MAJOR >= 0);
    QVERIFY(TINYORM_VERSION_MINOR >= 0);
    QVERIFY(TINYORM_VERSION_BUGFIX >= 0);
    QVERIFY(TINYORM_VERSION_BUILD >= 0);

    // Project and File Version strings
    QString versionStr = QString::number(TINYORM_VERSION_MAJOR) + QChar('.') +
                         QString::number(TINYORM_VERSION_MINOR) + QChar('.') +
                         QString::number(TINYORM_VERSION_BUGFIX);
    QString fileVersionStr = versionStr + QChar('.') +
                             QString::number(TINYORM_VERSION_BUILD);
    if constexpr (TINYORM_VERSION_BUILD > 0)
        versionStr += QChar('.') + QString::number(TINYORM_VERSION_BUILD);
    versionStr += TINYORM_VERSION_STATUS;

    QCOMPARE(TINYORM_FILEVERSION_STR, fileVersionStr);
    QCOMPARE(TINYORM_VERSION_STR, versionStr);
    QCOMPARE(TINYORM_VERSION_STR_2, QChar('v') + versionStr);

    // Project Version number, to check API compatibility
    const auto version = TINYORM_VERSION_MAJOR * 10000 +
                         TINYORM_VERSION_MINOR * 100 +
                         TINYORM_VERSION_BUGFIX;
    QCOMPARE(TINYORM_VERSION, version);
}

void tst_Version::versions_TinyUtils() const
{
    // Test types
    QCOMPARE(typeid (TINYUTILS_VERSION_MAJOR), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_MINOR), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_BUGFIX), typeid (int));
    QCOMPARE(typeid (TINYUTILS_VERSION_BUILD), typeid (int));

    // Individual version numbers have to be greater than zero
    QVERIFY(TINYUTILS_VERSION_MAJOR >= 0);
    QVERIFY(TINYUTILS_VERSION_MINOR >= 0);
    QVERIFY(TINYUTILS_VERSION_BUGFIX >= 0);
    QVERIFY(TINYUTILS_VERSION_BUILD >= 0);

    // Project and File Version strings
    QString versionStr = QString::number(TINYUTILS_VERSION_MAJOR) + QChar('.') +
                         QString::number(TINYUTILS_VERSION_MINOR) + QChar('.') +
                         QString::number(TINYUTILS_VERSION_BUGFIX);
    QString fileVersionStr = versionStr + QChar('.') +
                             QString::number(TINYUTILS_VERSION_BUILD);
    if constexpr (TINYUTILS_VERSION_BUILD > 0)
        versionStr += QChar('.') + QString::number(TINYUTILS_VERSION_BUILD);

    QCOMPARE(TINYUTILS_FILEVERSION_STR, fileVersionStr);
    QCOMPARE(TINYUTILS_VERSION_STR, versionStr);
    QCOMPARE(TINYUTILS_VERSION_STR_2, QChar('v') + versionStr);

    // Project Version number, to check API compatibility
    const auto version = TINYUTILS_VERSION_MAJOR * 10000 +
                         TINYUTILS_VERSION_MINOR * 100 +
                         TINYUTILS_VERSION_BUGFIX;
    QCOMPARE(TINYUTILS_VERSION, version);
}

void tst_Version::checkFileVersion_TinyOrm() const
{
#if !defined(_MSC_VER)
    QSKIP("checkFileVersion_*() related tests are supported on MSVC only.", );
#endif

    const auto filePath = QStringLiteral("%1/src/debug/TinyOrm%2.dll")
                          .arg(TINYORM_BUILD_TREE)
                          .arg(TINYORM_VERSION_MAJOR);
    const auto fileVersions = getExeVersionString(Fs::absolutePath(filePath));

    // Project and File Version strings
    const QString versionStr = QString::number(TINYORM_VERSION_MAJOR) + QChar('.') +
                               QString::number(TINYORM_VERSION_MINOR) + QChar('.') +
                               QString::number(TINYORM_VERSION_BUGFIX) + QChar('.') +
                               QString::number(TINYORM_VERSION_BUILD);

    QCOMPARE(fileVersions.productVersion, versionStr);
    QCOMPARE(fileVersions.fileVersion, fileVersions.productVersion);
}

void tst_Version::checkFileVersion_TinyUtils() const
{
#if !defined(_MSC_VER)
    QSKIP("checkFileVersion_*() related tests are supported on MSVC only.", );
#endif

    const auto filePath = QStringLiteral("%1/tests/auto/utils/debug/utils%2.dll")
                          .arg(TINYORM_BUILD_TREE)
                          .arg(TINYUTILS_VERSION_MAJOR);
    const auto fileVersions = getExeVersionString(Fs::absolutePath(filePath));

    // Project and File Version strings
    const QString versionStr = QString::number(TINYUTILS_VERSION_MAJOR) + QChar('.') +
                               QString::number(TINYUTILS_VERSION_MINOR) + QChar('.') +
                               QString::number(TINYUTILS_VERSION_BUGFIX) + QChar('.') +
                               QString::number(TINYUTILS_VERSION_BUILD);

    QCOMPARE(fileVersions.productVersion, versionStr);
    QCOMPARE(fileVersions.fileVersion, fileVersions.productVersion);
}

tst_Version::FileVersions
tst_Version::getExeVersionString(const QString &fileName) const
{
    // first of all, GetFileVersionInfoSize
    DWORD dwHandle = 1;
    DWORD dwLen = GetFileVersionInfoSize(fileName.toStdWString().c_str(), &dwHandle);
    if (dwLen == 0) {
        qDebug() << "Error in GetFileVersionInfoSize().";
        return {};
    }

    // GetFileVersionInfo
    LPBYTE lpData = new BYTE[dwLen];
    if (!GetFileVersionInfo(fileName.toStdWString().c_str(), dwHandle, dwLen,
                            static_cast<LPVOID>(lpData))) {
        delete[] lpData;
        qDebug() << "Error in GetFileVersionInfo().";
        return {};
    }

    // VerQueryValue
    VS_FIXEDFILEINFO *lpBuffer = NULL;
    UINT uLen = 0;

    if (!VerQueryValue(lpData, QString("\\").toStdWString().c_str(),
                       reinterpret_cast<LPVOID *>(&lpBuffer), &uLen)
    ) {
        delete[] lpData;
        qDebug() << "Error in VerQueryValue().";
        return {};
    }

    return {
        // Product Version
        QStringLiteral("%1.%2.%3.%4")
                .arg((lpBuffer->dwProductVersionMS >> 16) & 0xffff)
                .arg(lpBuffer->dwProductVersionMS & 0xffff)
                .arg((lpBuffer->dwProductVersionLS >> 16) & 0xffff)
                .arg(lpBuffer->dwProductVersionLS & 0xffff),
        // File Version
        QStringLiteral("%1.%2.%3.%4")
                .arg((lpBuffer->dwFileVersionMS >> 16) & 0xffff)
                .arg(lpBuffer->dwFileVersionMS & 0xffff)
                .arg((lpBuffer->dwFileVersionLS >> 16) & 0xffff)
                .arg(lpBuffer->dwFileVersionLS & 0xffff),
    };
}

QTEST_MAIN(tst_Version)

#include "tst_version.moc"
