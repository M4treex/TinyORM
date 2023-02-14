#include <QCoreApplication>
#include <QtTest>

#include "orm/exceptions/searchpathemptyerror.hpp"
#include "orm/postgresconnection.hpp"
#include "orm/schema.hpp"
#include "orm/utils/type.hpp"

#include "databases.hpp"

using Orm::Constants::PUBLIC;
using Orm::Constants::search_path;

using Orm::Exceptions::SearchPathEmptyError;
using Orm::PostgresConnection;
using Orm::Schema;

using TypeUtils = Orm::Utils::Type;

using TestUtils::Databases;

class tst_PostgreSQL_SchemaBuilder_f : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void hasTable_NoSearchPath_InConfiguration() const;
    void hasTable_EmptySearchPath_InConfiguration_UnqualifiedTablename_ThrowException() const;
    void hasTable_EmptySearchPath_InConfiguration_QualifiedTablename() const;

// NOLINTNEXTLINE(readability-redundant-access-specifiers)
private:
    /*! Test case class name. */
    inline static const auto *ClassName = "tst_PostgreSQL_SchemaBuilder_f";

    /*! The Database Manager instance in the TinyUtils. */
    std::shared_ptr<Orm::DatabaseManager> m_dm;
};

/* private slots */

// NOLINTBEGIN(readability-convert-member-functions-to-static)
void tst_PostgreSQL_SchemaBuilder_f::initTestCase()
{
    m_dm = Databases::manager();
}

void tst_PostgreSQL_SchemaBuilder_f::hasTable_NoSearchPath_InConfiguration() const
{
    // Add a new database connection
    const auto connectionName =
            Databases::createConnectionTempFrom(
                Databases::POSTGRESQL, {ClassName, QString::fromUtf8(__func__)},
                {}, {search_path});

    if (!connectionName)
        QSKIP(TestUtils::AutoTestSkipped.arg(TypeUtils::classPureBasename(*this))
                                        .toUtf8().constData(), );

    // Verify
    const auto hasTable = Schema::on(*connectionName).hasTable(QStringLiteral("users"));

    /* This check is really weird, our implementation queries the PostgreSQL database
       using the 'show search_path' query to obtain a real search_path if
       the 'search_path' configuration option is not defined. Because of that I have to
       check if the database search_path starts with the 'public' schema or is empty
       and based on that do the QVERIFY().
       Because I can't know what is the default for the search_path on foreign DB, this
       secures that this test passes whatever search_path will be set to.
       This also makes auto-tests dependent on the 'public' schema. */
    const auto searchPath = dynamic_cast<PostgresConnection &>(
                                m_dm->connection(*connectionName))
                            .searchPath();

    if (PostgresConnection::isSearchPathEmpty(searchPath) ||
        searchPath.constFirst() != PUBLIC
    )
        QVERIFY(!hasTable);
    else
        QVERIFY(hasTable);

    // Restore
    QVERIFY(Databases::removeConnection(*connectionName));
}

void tst_PostgreSQL_SchemaBuilder_f::
     hasTable_EmptySearchPath_InConfiguration_UnqualifiedTablename_ThrowException() const
{
    // Add a new database connection
    const auto connectionName =
            Databases::createConnectionTempFrom(
                Databases::POSTGRESQL, {ClassName, QString::fromUtf8(__func__)},
                {{search_path, QStringLiteral("''")}});

    if (!connectionName)
        QSKIP(TestUtils::AutoTestSkipped.arg(TypeUtils::classPureBasename(*this))
                                        .toUtf8().constData(), );

    // Verify
    QVERIFY_EXCEPTION_THROWN(
                Schema::on(*connectionName).hasTable(QStringLiteral("users")),
                SearchPathEmptyError);

    // Restore
    QVERIFY(Databases::removeConnection(*connectionName));
}

void tst_PostgreSQL_SchemaBuilder_f::
     hasTable_EmptySearchPath_InConfiguration_QualifiedTablename() const
{
    // Add a new database connection
    const auto connectionName =
            Databases::createConnectionTempFrom(
                Databases::POSTGRESQL, {ClassName, QString::fromUtf8(__func__)},
                {{search_path, QStringLiteral("''")}});

    if (!connectionName)
        QSKIP(TestUtils::AutoTestSkipped.arg(TypeUtils::classPureBasename(*this))
                                        .toUtf8().constData(), );

    // Verify
    QVERIFY(Schema::on(*connectionName).hasTable(QStringLiteral("public.users")));

    // Restore
    QVERIFY(Databases::removeConnection(*connectionName));
}
// NOLINTEND(readability-convert-member-functions-to-static)

QTEST_MAIN(tst_PostgreSQL_SchemaBuilder_f)

#include "tst_postgresql_schemabuilder_f.moc"
