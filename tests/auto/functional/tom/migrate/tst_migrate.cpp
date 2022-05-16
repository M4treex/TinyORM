#include <QCoreApplication>
#include <QtTest>

#include "tom/application.hpp"
#include "tom/commands/migrations/statuscommand.hpp"

#include "databases.hpp"

#include "migrations/2014_10_12_000000_create_posts_table.hpp"
#include "migrations/2014_10_12_100000_add_factor_column_to_posts_table.hpp"
#include "migrations/2014_10_12_200000_create_properties_table.hpp"
#include "migrations/2014_10_12_300000_create_phones_table.hpp"

using TomApplication = Tom::Application;

using Tom::Commands::Migrations::StatusCommand;
using Tom::Constants::Migrate;
using Tom::Constants::MigrateInstall;
using Tom::Constants::MigrateRefresh;
using Tom::Constants::MigrateReset;
using Tom::Constants::MigrateRollback;
using Tom::Constants::MigrateStatus;

using TestUtils::Databases;

using namespace Migrations; // NOLINT(google-build-using-namespace)

class tst_Migrate : public QObject
{
    Q_OBJECT

public:
    /*! Alias for the test output row. */
    using StatusRow = StatusCommand::StatusRow;
    /*! Type used for comparing results of the status command. */
    using Status = std::vector<StatusRow>;

private slots:
    void initTestCase();
    void cleanup() const;

    void migrate() const;
    void migrate_Step() const;

    void reset() const;

    void rollback_OnMigrate() const;
    void rollback_OnMigrateWithStep() const;

    void rollback_Step_OnMigrate() const;
    void rollback_Step_OnMigrateWithStep() const;

    void refresh_OnMigrate() const;
    void refresh_OnMigrateWithStep() const;

    void refresh_Step() const;
    void refresh_StepMigrate() const;
    void refresh_Step_StepMigrate() const;

// NOLINTNEXTLINE(readability-redundant-access-specifiers)
private:
    /*! Prepare arguments and invoke runCommand(). */
    [[nodiscard]] int
    invokeCommand(const QString &name, std::vector<const char *> &&arguments = {}) const;
    /*! Create a tom application instance and invoke the given command. */
    int runCommand(int &argc, const std::vector<const char *> &argv) const;

    /*! Invoke the status command to obtain results. */
    inline int invokeTestStatusCommand() const;
    /*! Get result of the last status command. */
    Status status() const;
    /*! Create a status object for comparing with the result of the status(). */
    Status createStatus(std::initializer_list<StatusRow> rows) const;
    /*! Create a status object to be equal after complete rollback. */
    Status createResetStatus() const;

    /*! Prepare the migration database for running. */
    void prepareDatabase() const;

    /*! Connection name used in this test case. */
    QString m_connection {};

    /*! Migrations table name. */
    inline static const auto MigrationsTable = QStringLiteral("migrations_unit_testing");
};

/*! Alias for the test output row. */
using Status   = tst_Migrate::Status;
/*! Type used for comparing results of the status command. */
using StatusRow = tst_Migrate::StatusRow;

/* Extracted common code to re-use. */
namespace
{
    // Status
    const auto *Yes = "Yes";
    const auto *No  = "No";

    // Batches
    const auto *s_1  = "1";
    const auto *s_2  = "2";
    const auto *s_3  = "3";
    const auto *s_4  = "4";

    // Migration names
    const auto *
    s_2014_10_12_000000_create_posts_table =
            "2014_10_12_000000_create_posts_table";
    const auto *
    s_2014_10_12_100000_add_factor_column_to_posts_table =
            "2014_10_12_100000_add_factor_column_to_posts_table";
    const auto *
    s_2014_10_12_200000_create_properties_table =
            "2014_10_12_200000_create_properties_table";
    const auto *
    s_2014_10_12_300000_create_phones_table =
            "2014_10_12_300000_create_phones_table";

    // Fully migrated w/o --step
    const std::initializer_list<StatusRow>
    FullyMigrated = {
        {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
        {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_1},
        {Yes, s_2014_10_12_200000_create_properties_table,          s_1},
        {Yes, s_2014_10_12_300000_create_phones_table,              s_1},
    };

    // Fully migrated with --step
    const std::initializer_list<StatusRow>
    FullyStepMigrated = {
        {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
        {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_2},
        {Yes, s_2014_10_12_200000_create_properties_table,          s_3},
        {Yes, s_2014_10_12_300000_create_phones_table,              s_4},
    };

} // namespace

/* private slots */

void tst_Migrate::initTestCase()
{
    m_connection = Databases::createConnection(Databases::MYSQL);

    if (m_connection.isEmpty())
        QSKIP(QStringLiteral("%1 autotest skipped, environment variables "
                             "for '%2' connection have not been defined.")
              .arg("tst_Migrate", Databases::MYSQL).toUtf8().constData(), );

    /* Modify the migrate:status command to not output a status table to the console but
       instead return a result as the vector, this vector is then used for comparing
       results. */
    TomApplication::enableInUnitTests();

    // Prepare the migration database for running
    prepareDatabase();
}

void tst_Migrate::cleanup() const
{
    /* All test methods need this except for two of them (reset and I don't remember
       second), I will not implement special logic to skip this for these two methods. */
    {
        auto exitCode = invokeCommand(MigrateReset);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createResetStatus(), status());
    }
}

void tst_Migrate::migrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }
}

void tst_Migrate::migrate_Step() const
{
    {
        auto exitCode = invokeCommand(Migrate, {"--step"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyStepMigrated), status());
    }
}

void tst_Migrate::reset() const
{
    {
        auto exitCode = invokeCommand(MigrateReset);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createResetStatus(), status());
    }
}

void tst_Migrate::rollback_OnMigrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // rollback on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRollback);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createResetStatus(), status());
    }
}

void tst_Migrate::rollback_OnMigrateWithStep() const
{
    {
        auto exitCode = invokeCommand(Migrate, {"--step"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyStepMigrated), status());
    }

    // rollback on previous migrate with --step
    {
        auto exitCode = invokeCommand(MigrateRollback);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus({
            {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
            {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_2},
            {Yes, s_2014_10_12_200000_create_properties_table,          s_3},
            {No,  s_2014_10_12_300000_create_phones_table},
        }), status());
    }
}

void tst_Migrate::rollback_Step_OnMigrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // rollback on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRollback, {"--step=2"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus({
            {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
            {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_1},
            {No,  s_2014_10_12_200000_create_properties_table},
            {No,  s_2014_10_12_300000_create_phones_table},
        }), status());
    }
}

void tst_Migrate::rollback_Step_OnMigrateWithStep() const
{
    {
        auto exitCode = invokeCommand(Migrate, {"--step"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyStepMigrated), status());
    }

    // rollback on previous migrate with --step
    {
        auto exitCode = invokeCommand(MigrateRollback, {"--step=2"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus({
            {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
            {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_2},
            {No,  s_2014_10_12_200000_create_properties_table},
            {No,  s_2014_10_12_300000_create_phones_table},
        }), status());
    }
}

void tst_Migrate::refresh_OnMigrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // refresh on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRefresh);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }
}

void tst_Migrate::refresh_OnMigrateWithStep() const
{
    {
        auto exitCode = invokeCommand(Migrate, {"--step"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyStepMigrated), status());
    }

    // refresh on previous migrate with --step
    {
        auto exitCode = invokeCommand(MigrateRefresh);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }
}

void tst_Migrate::refresh_Step() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // refresh on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRefresh, {"--step=2"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus({
            {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
            {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_1},
            {Yes, s_2014_10_12_200000_create_properties_table,          s_2},
            {Yes, s_2014_10_12_300000_create_phones_table,              s_2},
        }), status());
    }
}

void tst_Migrate::refresh_StepMigrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // refresh on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRefresh, {"--step-migrate"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyStepMigrated), status());
    }
}

void tst_Migrate::refresh_Step_StepMigrate() const
{
    {
        auto exitCode = invokeCommand(Migrate);

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus(FullyMigrated), status());
    }

    // refresh on previous migrate w/o --step
    {
        auto exitCode = invokeCommand(MigrateRefresh, {"--step=2", "--step-migrate"});

        QVERIFY(exitCode == EXIT_SUCCESS);
    }

    {
        auto exitCode = invokeTestStatusCommand();

        QVERIFY(exitCode == EXIT_SUCCESS);
        QCOMPARE(createStatus({
            {Yes, s_2014_10_12_000000_create_posts_table,               s_1},
            {Yes, s_2014_10_12_100000_add_factor_column_to_posts_table, s_1},
            {Yes, s_2014_10_12_200000_create_properties_table,          s_2},
            {Yes, s_2014_10_12_300000_create_phones_table,              s_3},
        }), status());
    }
}

/* private */

int tst_Migrate::invokeCommand(const QString &name,
                               std::vector<const char *> &&arguments) const
{
    static const auto connectionTmpl = QStringLiteral("--database=%1");

    // Prepare fake argc and argv
    const auto nameArr = name.toUtf8();
    // FUTURE tests tom, when the schema builder will support more db drivers, I can run it on all supported connections, code will look like in the tst_querybuilder.cpp, then I will fetch connection name in every test method using QFETCH_GLOBAL() and I will pass this connection name to the invokeCommand(), so I will discard m_connection and will use method parameter connection here silverqx
    /* Schema builder is implemented only for the MySQL driver, so I can use m_connection
       here as the default connection. */
    // DB connection to use
    const auto connectionArr = connectionTmpl.arg(m_connection).toUtf8();

    std::vector<const char *> argv {
#ifdef _WIN32
        "tom.exe",
#else
        "tom",
#endif
        nameArr.constData(),
        connectionArr.constData(),
//        "-vvv",
    };
    std::ranges::move(arguments, std::back_inserter(argv));

    int argc = static_cast<int>(argv.size());

    return runCommand(argc, argv);
}

int tst_Migrate::runCommand(int &argc, const std::vector<const char *> &argv) const
{
    try {
        // env. should be always development so passed {} for env. name
        return TomApplication(argc, const_cast<char **>(argv.data()),
                              Databases::manager(), "TOM_TESTS_ENV", MigrationsTable)
                .migrations<CreatePostsTable,
                            AddFactorColumnToPostsTable,
                            CreatePropertiesTable,
                            CreatePhonesTable>()
                // Fire it up 🔥🚀✨
                .runWithArguments({argv.cbegin(), argv.cend()});

    } catch (const std::exception &e) {

        TomApplication::logException(e, true);
    }

    return EXIT_FAILURE;
}

int tst_Migrate::invokeTestStatusCommand() const
{
    return invokeCommand(MigrateStatus);
}

Status tst_Migrate::status() const
{
    return TomApplication::status();
}

Status tst_Migrate::createStatus(std::initializer_list<StatusRow> rows) const
{
    return rows;
}

Status tst_Migrate::createResetStatus() const
{
    return {
        {No, s_2014_10_12_000000_create_posts_table},
        {No, s_2014_10_12_100000_add_factor_column_to_posts_table},
        {No, s_2014_10_12_200000_create_properties_table},
        {No, s_2014_10_12_300000_create_phones_table},
    };
}

void tst_Migrate::prepareDatabase() const
{
    // Ownership of a unique_ptr()
    const auto schema = Databases::manager()->connection(m_connection)
                        .getSchemaBuilder();

    // Create the migrations table if needed
    if (!schema->hasTable(MigrationsTable)) {
        auto exitCode = invokeCommand(MigrateInstall);

        QVERIFY(exitCode == EXIT_SUCCESS);

        return;
    }

    // Reset the migrations table
    auto exitCode = invokeCommand(MigrateReset);

    QVERIFY(exitCode == EXIT_SUCCESS);
}

QTEST_MAIN(tst_Migrate)

#include "tst_migrate.moc"
