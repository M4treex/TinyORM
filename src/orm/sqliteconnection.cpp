#include "orm/sqliteconnection.hpp"

#include "orm/query/grammars/sqlitegrammar.hpp"
#include "orm/query/processors/sqliteprocessor.hpp"
#include "orm/schema/grammars/sqlitegrammar.hpp"
#include "orm/schema/sqlitebuilder.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm
{

SQLiteConnection::SQLiteConnection(
        const std::function<Connectors::ConnectionName()> &connection,
        const QString &database, const QString tablePrefix,
        const QVariantHash &config
)
    : DatabaseConnection(connection, database, tablePrefix, config)
{
    /* We need to initialize a query grammar that is a very important part
       of the database abstraction, so we initialize it to the default value
       while starting. */
    useDefaultQueryGrammar();

    useDefaultPostProcessor();
}

std::unique_ptr<SchemaBuilder> SQLiteConnection::getSchemaBuilder()
{
    if (!m_schemaGrammar)
        useDefaultSchemaGrammar();

    return std::make_unique<Schema::SQLiteBuilder>(*this);
}

std::unique_ptr<QueryGrammar> SQLiteConnection::getDefaultQueryGrammar() const
{
    // FEATURE table prefix silverqx
    return std::make_unique<Query::Grammars::SQLiteGrammar>();
}

std::unique_ptr<SchemaGrammar> SQLiteConnection::getDefaultSchemaGrammar() const
{
    // FEATURE table prefix silverqx
    return std::make_unique<Schema::Grammars::SQLiteGrammar>();
}

std::unique_ptr<QueryProcessor> SQLiteConnection::getDefaultPostProcessor() const
{
    return std::make_unique<Query::Processors::SQLiteProcessor>();
}

} // namespace Orm
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif
