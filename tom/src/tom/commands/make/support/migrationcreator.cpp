#include "tom/commands/make/support/migrationcreator.hpp"

#include <QDateTime>

#include <fstream>

#include <orm/tiny/utils/string.hpp>

#include "tom/commands/make/stubs/migrationstubs.hpp"
#include "tom/tomconstants.hpp"

using fspath = std::filesystem::path;

using StringUtils = Orm::Tiny::Utils::String;

using Tom::Commands::Make::Stubs::MigrationCreateStub;
using Tom::Commands::Make::Stubs::MigrationUpdateStub;
using Tom::Commands::Make::Stubs::MigrationStub;
using Tom::Constants::DateTimePrefix;

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Make::Support
{

/* public */

fspath MigrationCreator::create(
            std::string &&datetimePrefix, const QString &name, std::string &&extension,
            fspath &&migrationsPath, const QString &table, const bool create) const
{
    auto migrationPath = getPath(std::move(datetimePrefix), name, std::move(extension),
                                 migrationsPath);

    /* First we will get the stub file for the migration, which serves as a type
       of template for the migration. Once we have those we will populate the
       various place-holders, and save the file. */
    auto stub = getStub(table, create);

    // Output it as binary stream to force line endings to LF
    std::ofstream(migrationPath, std::ios::out | std::ios::binary)
            << populateStub(name, std::move(stub), table);

    return migrationPath;
}

/* protected */

QString MigrationCreator::getStub(const QString &table, const bool create)
{
    QString stub;

    if (table.isEmpty())
        stub = MigrationStub;

    else if (create)
        stub = MigrationCreateStub;

    else
        stub = MigrationUpdateStub;

    return stub;
}

fspath MigrationCreator::getPath(std::string &&datetimePrefix, const QString &name,
                                 std::string &&extension, const fspath &path) const
{
    std::string filename = datetimePrefix.empty() ? getDatePrefix()
                                                  : std::move(datetimePrefix);

    filename += '_' + name.toStdString();

    filename += extension.empty() ? ".hpp" : std::move(extension);

    return path / std::move(filename);
}

std::string MigrationCreator::getDatePrefix()
{
    return QDateTime::currentDateTime().toString(DateTimePrefix).toStdString();
}

std::string
MigrationCreator::populateStub(const QString &name, QString &&stub, const QString &table)
{
    const auto className = getClassName(name);

    stub.replace(QStringLiteral("DummyClass"), className)
        .replace(QStringLiteral("{{ class }}"), className)
        .replace(QStringLiteral("{{class}}"), className);

    /* Here we will replace the table place-holders with the table specified by
       the developer, which is useful for quickly creating a tables creation
       or update migration from the console instead of typing it manually. */
    if (!table.isEmpty())
        stub.replace(QStringLiteral("DummyTable"), table)
            .replace(QStringLiteral("{{ table }}"), table)
            .replace(QStringLiteral("{{table}}"), table);

    return stub.toStdString();
}

QString MigrationCreator::getClassName(const QString &name)
{
    return StringUtils::studly(name);
}

} // namespace Tom::Commands::Make::Support

TINYORM_END_COMMON_NAMESPACE
