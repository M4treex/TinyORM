#ifndef CONNECTIONINTERFACE_HPP
#define CONNECTIONINTERFACE_HPP

#include <QtSql/QSqlQuery>

#include "orm/query/expression.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm
{

    class DatabaseConnection;

namespace Query
{
    class Builder;

namespace Grammars
{
    class Grammar;
}
namespace Processors
{
    class Processor;
}
} // Orm::Query

namespace Schema
{
    class SchemaBuilder;
namespace Grammars
{
    class SchemaGrammar;
}
} // Orm::Schema

    using QueryBuilder   = Query::Builder;
    using QueryGrammar   = Query::Grammars::Grammar;
    using QueryProcessor = Query::Processors::Processor;
    using SchemaBuilder  = Schema::SchemaBuilder;
    using SchemaGrammar  = Schema::Grammars::SchemaGrammar;

    /*! Counts executed statements in a current connection. */
    struct StatementsCounter
    {
        /*! Normal select statements. */
        int normal = -1;
        /*! Affecting statements (UPDATE, INSERT, DELETE). */
        int affecting = -1;
        /*! Transactional statements (START TRANSACTION, ROLLBACK, COMMIT, SAVEPOINT). */
        int transactional = -1;
    };

    class ConnectionInterface
    {
    public:
        inline virtual ~ConnectionInterface() = default;

        /*! Begin a fluent query against a database table. */
        virtual QSharedPointer<QueryBuilder>
        table(const QString &table, const QString &as = "") = 0;

        /*! Get a new query builder instance. */
        virtual QSharedPointer<QueryBuilder> query() = 0;

        /*! Get a new raw query expression. */
        virtual Query::Expression raw(const QVariant &value) const = 0;

        /*! Execute a Closure within a transaction. */
//        public function transaction(Closure $callback, $attempts = 1);

        /*! Start a new database transaction. */
        virtual bool beginTransaction() = 0;

        /*! Commit the active database transaction. */
        virtual bool commit() = 0;

        /*! Rollback the active database transaction. */
        virtual bool rollBack() = 0;

        /*! Start a new named transaction savepoint. */
        virtual bool savepoint(const QString &id) = 0;

        /*! Start a new named transaction savepoint. */
        virtual bool savepoint(size_t id) = 0;

        /*! Rollback to a named transaction savepoint. */
        virtual bool rollbackToSavepoint(const QString &id) = 0;

        /*! Rollback to a named transaction savepoint. */
        virtual bool rollbackToSavepoint(size_t id) = 0;

        /*! Get the number of active transactions. */
        virtual uint transactionLevel() const = 0;

        /*! Run a select statement and return a single result. */
        virtual QSqlQuery
        selectOne(const QString &queryString,
                  const QVector<QVariant> &bindings = {}) = 0;

        /*! Run a select statement against the database. */
        virtual QSqlQuery
        select(const QString &queryString,
               const QVector<QVariant> &bindings = {}) = 0;

        /*! Run a select statement against the database. */
        virtual QSqlQuery
        selectFromWriteConnection(const QString &queryString,
                                  const QVector<QVariant> &bindings = {}) = 0;

        /*! Run a select statement against the database and returns a generator. */
//        public function cursor($query, $bindings = [], $useReadPdo = true);

        /*! Run an insert statement against the database. */
        virtual QSqlQuery
        insert(const QString &queryString,
               const QVector<QVariant> &bindings = {}) = 0;

        /*! Run an update statement against the database. */
        virtual std::tuple<int, QSqlQuery>
        update(const QString &queryString,
               const QVector<QVariant> &bindings = {}) = 0;

        /*! Run a delete statement against the database. */
        virtual std::tuple<int, QSqlQuery>
        remove(const QString &queryString,
               const QVector<QVariant> &bindings = {}) = 0;

        /*! Execute an SQL statement and return the boolean result. */
        virtual QSqlQuery
        statement(const QString &queryString,
                  const QVector<QVariant> &bindings = {}) = 0;

        /*! Run an SQL statement and get the number of rows affected. */
        virtual std::tuple<int, QSqlQuery>
        affectingStatement(const QString &queryString,
                           const QVector<QVariant> &bindings = {}) = 0;

        /*! Get a new QSqlQuery instance for the current connection. */
        virtual QSqlQuery getQtQuery() = 0;

        /*! Prepare the query bindings for execution. */
        virtual QVector<QVariant>
        prepareBindings(QVector<QVariant> bindings) const = 0;

        /*! Check database connection and show warnings when the state changed. */
        virtual bool pingDatabase() = 0;

        // FEATURE pretend, implement dry run mode silverqx
        /*! Execute the given callback in "dry run" mode. */
//        public function pretend(Closure $callback);

        /*! Get the database connection name. */
        virtual const QString getName() const = 0;

        /*! Get the name of the connected database. */
        virtual const QString &getDatabaseName() const = 0;

        /*! Set the query grammar to the default implementation. */
        virtual void useDefaultQueryGrammar() = 0;
        /*! Get the query grammar used by the connection. */
        virtual const QueryGrammar &getQueryGrammar() const = 0;

        /*! Set the schema grammar to the default implementation. */
        virtual void useDefaultSchemaGrammar() = 0;
        /*! Get the schema grammar used by the connection. */
        virtual const SchemaGrammar &getSchemaGrammar() const = 0;

        /*! Get a schema builder instance for the connection. */
        virtual std::unique_ptr<SchemaBuilder> getSchemaBuilder() = 0;

        /*! Set the query post processor to the default implementation. */
        virtual void useDefaultPostProcessor() = 0;
        /*! Get the query post processor used by the connection. */
        virtual const QueryProcessor &getPostProcessor() const = 0;

        /* Queries execution time counter */
        /*! Determine whether we're counting queries execution time. */
        virtual bool countingElapsed() const = 0;
        /*! Enable counting queries execution time on the current connection. */
        virtual DatabaseConnection &enableElapsedCounter() = 0;
        /*! Disable counting queries execution time on the current connection. */
        virtual DatabaseConnection &disableElapsedCounter() = 0;
        /*! Obtain queries execution time. */
        virtual qint64 getElapsedCounter() const = 0;
        /*! Obtain and reset queries execution time. */
        virtual qint64 takeElapsedCounter() = 0;
        /*! Reset queries execution time. */
        virtual DatabaseConnection &resetElapsedCounter() = 0;

        /* Queries executed counter */
        /*! Determine whether we're counting the number of executed queries. */
        virtual bool countingStatements() const = 0;
        /*! Enable counting the number of executed queries on the current connection. */
        virtual DatabaseConnection &enableStatementsCounter() = 0;
        /*! Disable counting the number of executed queries on the current connection. */
        virtual DatabaseConnection &disableStatementsCounter() = 0;
        /*! Obtain the number of executed queries. */
        virtual const StatementsCounter &getStatementsCounter() const = 0;
        /*! Obtain and reset the number of executed queries. */
        virtual StatementsCounter takeStatementsCounter() = 0;
        /*! Reset the number of executed queries. */
        virtual DatabaseConnection &resetStatementsCounter() = 0;

        /*! Return the connection's driver name. */
        virtual QString driverName() = 0;
    };

} // namespace Orm
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif

#endif // CONNECTIONINTERFACE_HPP
