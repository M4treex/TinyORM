#ifndef RELATION_H
#define RELATION_H

#include <QtSql/QSqlQuery>

#include <optional>

#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>

#include "orm/logicerror.hpp"
#include "orm/ormtypes.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm
{
namespace Query
{
    class JoinClause;
}
namespace Tiny
{
    template<class Model>
    class Builder;

    template<class Model, typename ...AllRelations>
    class BaseModel;

namespace Relations
{

    template<class Model, class Related>
    class Relation
    {
        using JoinClause = Orm::Query::JoinClause;

    protected:
        Relation(std::unique_ptr<Related> &&related, Model &parent);

    public:
        inline virtual ~Relation() = default;

        /*! Set the base constraints on the relation query. */
        virtual void addConstraints() const = 0;

        /*! Run a callback with constraints disabled on the relation. */
        static std::unique_ptr<Relation<Model, Related>>
        noConstraints(const std::function<
                      std::unique_ptr<Relation<Model, Related>>()> &callback);

        /*! Set the constraints for an eager load of the relation. */
        virtual void addEagerConstraints(const QVector<Model> &models) const = 0;
        /*! Initialize the relation on a set of models. */
        virtual QVector<Model> &
        initRelation(QVector<Model> &models, const QString &relation) const = 0;
        /*! Match the eagerly loaded results to their parents. */
        virtual void match(QVector<Model> &models, QVector<Related> results,
                           const QString &relation) const = 0;
        /*! Get the results of the relationship. */
        virtual std::variant<QVector<Related>, std::optional<Related>>
        getResults() const = 0;

        /*! Get the relationship for eager loading. */
        inline QVector<Related> getEager() const
        { return get(); }
        /*! Execute the query as a "select" statement. */
        inline virtual QVector<Related> get(const QStringList &columns = {"*"}) const
        { return m_query->get(columns); }

        /* Getters / Setters */
        /*! Get the underlying query for the relation. */
        inline Builder<Related> &getQuery() const
        { return *m_query; }
        /*! Get the base query builder driving the Eloquent builder. */
        inline QueryBuilder &getBaseQuery() const
        { return m_query->getQuery(); }

        /*! Get the parent model of the relation. */
        const Model &getParent() const
        { return m_parent; }
        /*! Get the related model of the relation. */
        const Related &getRelated() const
        { return *m_related; }
        /*! Get the name of the "created at" column. */
        const QString &createdAt() const
        { return m_parent.getCreatedAtColumn(); }
        /*! Get the name of the "updated at" column. */
        const QString &updatedAt() const
        { return m_parent.getUpdatedAtColumn(); }
        /*! Get the name of the related model's "updated at" column. */
        const QString &relatedUpdatedAt() const
        { return m_related->getUpdatedAtColumn(); }

        /* Others */
        /*! Touch all of the related models for the relationship. */
        virtual void touch() const;
        /*! Run a raw update against the base query. */
        std::tuple<int, QSqlQuery>
        rawUpdate(const QVector<UpdateItem> &values = {}) const;

        /* TinyBuilder proxy methods */
        std::optional<Related>
        find(const QVariant &id, const QStringList &columns = {"*"}) const;
        /*! Find a model by its primary key or return fresh model instance. */
        Related findOrNew(const QVariant &id, const QStringList &columns = {"*"}) const;
        /*! Find a model by its primary key or throw an exception. */
        Related findOrFail(const QVariant &id, const QStringList &columns = {"*"}) const;

        /*! Get the first record matching the attributes or instantiate it. */
        Related firstOrNew(const QVector<WhereItem> &attributes = {},
                         const QVector<AttributeItem> &values = {}) const;
        /*! Get the first record matching the attributes or create it. */
        Related firstOrCreate(const QVector<WhereItem> &attributes = {},
                            const QVector<AttributeItem> &values = {}) const;
        /*! Execute the query and get the first result or throw an exception. */
        Related firstOrFail(const QStringList &columns = {"*"}) const;

        /*! Add a basic where clause to the query, and return the first result. */
        std::optional<Related>
        firstWhere(const QString &column, const QString &comparison,
                   const QVariant &value, const QString &condition = "and") const;
        /*! Add a basic where clause to the query, and return the first result. */
        std::optional<Related>
        firstWhereEq(const QString &column, const QVariant &value,
                     const QString &condition = "and") const;

        /*! Get a single column's value from the first result of a query. */
        QVariant value(const QString &column) const;

        /*! Set the relationships that should be eager loaded. */
        Builder<Related> &with(const QVector<WithItem> &relations) const;
        /*! Set the relationships that should be eager loaded. */
        Builder<Related> &with(const QString &relation) const;
        /*! Prevent the specified relations from being eager loaded. */
        Builder<Related> &without(const QVector<QString> &relations) const;
        /*! Prevent the specified relations from being eager loaded. */
        Builder<Related> &without(const QString &relation) const;

        /*! Save a new model and return the instance. */
        virtual Related create(const QVector<AttributeItem> &attributes) const;
        /*! Save a new model and return the instance. */
        virtual Related create(QVector<AttributeItem> &&attributes) const;

        /* Proxies to TinyBuilder -> BuildsQueries */
        /*! Execute the query and get the first result. */
        std::optional<Model> first(const QStringList &columns = {"*"}) const;

        /* Proxies to TinyBuilder -> QueryBuilder */
        /* Insert, Update, Delete */
        /*! Insert new records into the database. */
        std::tuple<bool, std::optional<QSqlQuery>>
        insert(const QVector<AttributeItem> &attributes) const;
        /*! Insert a new record and get the value of the primary key. */
        quint64 insertGetId(const QVector<AttributeItem> &attributes) const;

        /*! Update records in the database. */
        std::tuple<int, QSqlQuery>
        update(const QVector<UpdateItem> &values) const;
        /*! Create or update a record matching the attributes, and fill it with values. */
        Related updateOrCreate(const QVector<WhereItem> &attributes,
                               const QVector<AttributeItem> &values = {}) const;

        /*! Delete records from the database. */
        std::tuple<int, QSqlQuery> remove() const;
        /*! Delete records from the database. */
        std::tuple<int, QSqlQuery> deleteModels() const;

        /*! Run a truncate statement on the table. */
        void truncate() const;

        /* Select */
        /*! Set the columns to be selected. */
        Builder<Related> &select(const QStringList columns = {"*"}) const;
        /*! Set the column to be selected. */
        Builder<Related> &select(const QString column) const;
        /*! Add new select columns to the query. */
        Builder<Related> &addSelect(const QStringList &columns) const;
        /*! Add a new select column to the query. */
        Builder<Related> &addSelect(const QString &column) const;

        /*! Force the query to only return distinct results. */
        Builder<Related> &distinct() const;

        /*! Add a join clause to the query. */
        Builder<Related> &join(const QString &table, const QString &first,
                               const QString &comparison, const QString &second,
                               const QString &type = "inner", bool where = false) const;
        /*! Add an advanced join clause to the query. */
        Builder<Related> &join(const QString &table,
                               const std::function<void(JoinClause &)> &callback,
                               const QString &type = "inner") const;
        /*! Add a "join where" clause to the query. */
        Builder<Related> &joinWhere(const QString &table, const QString &first,
                                    const QString &comparison, const QString &second,
                                    const QString &type = "inner") const;
        /*! Add a left join to the query. */
        Builder<Related> &leftJoin(
                const QString &table, const QString &first,
                const QString &comparison, const QString &second) const;
        /*! Add an advanced left join to the query. */
        Builder<Related> &leftJoin(
                const QString &table,
                const std::function<void(JoinClause &)> &callback) const;
        /*! Add a "join where" clause to the query. */
        Builder<Related> &leftJoinWhere(const QString &table, const QString &first,
                                        const QString &comparison,
                                        const QString &second) const;
        /*! Add a right join to the query. */
        Builder<Related> &rightJoin(
                const QString &table, const QString &first,
                const QString &comparison, const QString &second) const;
        /*! Add an advanced right join to the query. */
        Builder<Related> &rightJoin(
                const QString &table,
                const std::function<void(JoinClause &)> &callback) const;
        /*! Add a "right join where" clause to the query. */
        Builder<Related> &rightJoinWhere(const QString &table, const QString &first,
                                         const QString &comparison,
                                         const QString &second) const;
        /*! Add a "cross join" clause to the query. */
        Builder<Related> &crossJoin(
                const QString &table, const QString &first,
                const QString &comparison, const QString &second) const;
        /*! Add an advanced "cross join" clause to the query. */
        Builder<Related> &crossJoin(
                const QString &table,
                const std::function<void(JoinClause &)> &callback) const;

        /*! Add a basic where clause to the query. */
        Builder<Related> &where(
                const QString &column, const QString &comparison,
                const QVariant &value, const QString &condition = "and") const;
        /*! Add an "or where" clause to the query. */
        Builder<Related> &orWhere(const QString &column, const QString &comparison,
                                  const QVariant &value) const;
        /*! Add a basic equal where clause to the query. */
        Builder<Related> &whereEq(const QString &column, const QVariant &value,
                                  const QString &condition = "and") const;
        /*! Add an equal "or where" clause to the query. */
        Builder<Related> &orWhereEq(const QString &column, const QVariant &value) const;
        /*! Add a nested where clause to the query. */
        Builder<Related> &where(const std::function<void(Builder<Related> &)> &callback,
                                const QString &condition = "and") const;
        /*! Add a nested "or where" clause to the query. */
        Builder<Related> &orWhere(
                const std::function<void(Builder<Related> &)> &callback) const;

        /*! Add a vector of basic where clauses to the query. */
        Builder<Related> &where(const QVector<WhereItem> &values,
                                const QString &condition = "and") const;
        /*! Add a vector of basic "or where" clauses to the query. */
        Builder<Related> &orWhere(const QVector<WhereItem> &values) const;

        /*! Add a vector of where clauses comparing two columns to the query. */
        Builder<Related> &whereColumn(const QVector<WhereColumnItem> &values,
                                      const QString &condition = "and") const;
        /*! Add a vector of "or where" clauses comparing two columns to the query. */
        Builder<Related> &orWhereColumn(const QVector<WhereColumnItem> &values) const;

        /*! Add a "where" clause comparing two columns to the query. */
        Builder<Related> &whereColumn(const QString &first, const QString &comparison,
                                      const QString &second,
                                      const QString &condition = "and") const;
        /*! Add a "or where" clause comparing two columns to the query. */
        Builder<Related> &orWhereColumn(const QString &first, const QString &comparison,
                                        const QString &second) const;
        /*! Add an equal "where" clause comparing two columns to the query. */
        Builder<Related> &whereColumnEq(const QString &first, const QString &second,
                                        const QString &condition = "and") const;
        /*! Add an equal "or where" clause comparing two columns to the query. */
        Builder<Related> &orWhereColumnEq(const QString &first,
                                          const QString &second) const;

        /*! Add a "where in" clause to the query. */
        Builder<Related> &whereIn(
                const QString &column, const QVector<QVariant> &values,
                const QString &condition = "and", bool nope = false) const;
        /*! Add an "or where in" clause to the query. */
        Builder<Related> &orWhereIn(const QString &column,
                                    const QVector<QVariant> &values) const;
        /*! Add a "where not in" clause to the query. */
        Builder<Related> &whereNotIn(const QString &column,
                                     const QVector<QVariant> &values,
                                     const QString &condition = "and") const;
        /*! Add an "or where not in" clause to the query. */
        Builder<Related> &orWhereNotIn(const QString &column,
                                       const QVector<QVariant> &values) const;

        /*! Add a "where null" clause to the query. */
        Builder<Related> &whereNull(const QStringList &columns = {"*"},
                                    const QString &condition = "and",
                                    bool nope = false) const;
        /*! Add a "where null" clause to the query. */
        Builder<Related> &whereNull(const QString &column,
                                    const QString &condition = "and",
                                    bool nope = false) const;
        /*! Add an "or where null" clause to the query. */
        Builder<Related> &orWhereNull(const QStringList &columns = {"*"}) const;
        /*! Add an "or where null" clause to the query. */
        Builder<Related> &orWhereNull(const QString &column) const;
        /*! Add a "where not null" clause to the query. */
        Builder<Related> &whereNotNull(const QStringList &columns = {"*"},
                                       const QString &condition = "and") const;
        /*! Add a "where not null" clause to the query. */
        Builder<Related> &whereNotNull(const QString &column,
                                       const QString &condition = "and") const;
        /*! Add an "or where not null" clause to the query. */
        Builder<Related> &orWhereNotNull(const QStringList &columns = {"*"}) const;
        /*! Add an "or where not null" clause to the query. */
        Builder<Related> &orWhereNotNull(const QString &column) const;

        /*! Add a "group by" clause to the query. */
        Builder<Related> &groupBy(const QStringList &groups) const;
        /*! Add a "group by" clause to the query. */
        Builder<Related> &groupBy(const QString &group) const;

        /*! Add a "having" clause to the query. */
        Builder<Related> &having(const QString &column, const QString &comparison,
                                 const QVariant &value,
                                 const QString &condition = "and") const;
        /*! Add an "or having" clause to the query. */
        Builder<Related> &orHaving(const QString &column, const QString &comparison,
                                   const QVariant &value) const;

        /*! Add an "order by" clause to the query. */
        Builder<Related> &orderBy(const QString &column,
                                  const QString &direction = "asc") const;
        /*! Add a descending "order by" clause to the query. */
        Builder<Related> &orderByDesc(const QString &column) const;
        /*! Add an "order by" clause for a timestamp to the query. */
        Builder<Related> &latest(const QString &column = "") const;
        /*! Add an "order by" clause for a timestamp to the query. */
        Builder<Related> &oldest(const QString &column = "") const;
        /*! Remove all existing orders. */
        Builder<Related> &reorder() const;
        /*! Remove all existing orders and optionally add a new order. */
        Builder<Related> &reorder(const QString &column,
                                  const QString &direction = "asc") const;

        /*! Set the "limit" value of the query. */
        Builder<Related> &limit(int value) const;
        /*! Alias to set the "limit" value of the query. */
        Builder<Related> &take(int value) const;
        /*! Set the "offset" value of the query. */
        Builder<Related> &offset(int value) const;
        /*! Alias to set the "offset" value of the query. */
        Builder<Related> &skip(int value) const;
        /*! Set the limit and offset for a given page. */
        Builder<Related> &forPage(int page, int perPage = 30) const;

        /*! Increment a column's value by a given amount. */
        template<typename T> requires std::is_arithmetic_v<T>
        std::tuple<int, QSqlQuery>
        increment(const QString &column, T amount = 1,
                  const QVector<UpdateItem> &extra = {}) const;
        /*! Decrement a column's value by a given amount. */
        template<typename T> requires std::is_arithmetic_v<T>
        std::tuple<int, QSqlQuery>
        decrement(const QString &column, T amount = 1,
                  const QVector<UpdateItem> &extra = {}) const;

        /*! Add a where clause on the primary key to the query. */
        Builder<Related> &whereKey(const QVariant &id) const;
        /*! Add a where clause on the primary key to the query. */
        Builder<Related> &whereKeyNot(const QVariant &id) const;

        /* Inserting operations on the relationship */
        // TODO study, would be possible to disable this by SFINAE by current class type? eg std::enable_if this is ManyRelation or PivotRelation; and if yes, then it's a good idea do it this way? silverqx
        /*! Attach a model instance to the parent model. */
        inline virtual std::tuple<bool, Related &> save(Related &) const
        { throw LogicError(methodNotImplementedMessage("save")); }
        /*! Attach a model instance to the parent model. */
        inline virtual std::tuple<bool, Related> save(Related &&) const
        { throw LogicError(methodNotImplementedMessage("save")); }
        /*! Attach a vector of models to the parent instance. */
        inline virtual QVector<Related> &saveMany(QVector<Related> &) const
        { throw LogicError(methodNotImplementedMessage("saveMany")); }
        /*! Attach a vector of models to the parent instance. */
        inline virtual QVector<Related> saveMany(QVector<Related> &&) const
        { throw LogicError(methodNotImplementedMessage("saveMany")); }

        /*! Create a vector of new instances of the related model. */
        inline virtual QVector<Related>
        createMany(const QVector<QVector<AttributeItem>> &) const
        { throw LogicError(methodNotImplementedMessage("createMany")); }
        /*! Create a vector of new instances of the related model. */
        inline virtual QVector<Related>
        createMany(QVector<QVector<AttributeItem>> &&) const
        { throw LogicError(methodNotImplementedMessage("createMany")); }

        /* Updating relationship */
        /*! Associate the model instance to the given parent. */
        inline virtual Model &associate(const Related &) const
        { throw LogicError(methodNotImplementedMessage("associate")); }
        /*! Associate the model instance to the given parent. */
        inline virtual Model &associate(const QVariant &) const
        { throw LogicError(methodNotImplementedMessage("associate")); }
        /*! Dissociate previously associated model from the given parent. */
        inline virtual Model &dissociate() const
        { throw LogicError(QStringLiteral(
                               "The 'dissociate' or an alias 'disassociate' method "
                               "is not implemented for the '%1' relation type.")
                           .arg(relationTypeName())); }
        /*! Alias of "dissociate" method. */
        inline virtual Model &disassociate() const
        { return dissociate(); }

        /* Many-To-Many */
        /*! Attach models to the parent. */
        inline virtual void attach(const QVector<QVariant> &,
                                   const QVector<AttributeItem> & = {},
                                   bool = true) const
        { throw LogicError(methodNotImplementedMessage("attach")); }
        /*! Attach models to the parent. */
        inline virtual void attach(const QVector<std::reference_wrapper<Related>> &,
                                   const QVector<AttributeItem> & = {},
                                   bool = true) const
        { throw LogicError(methodNotImplementedMessage("attach")); }
        /*! Attach a model to the parent. */
        inline virtual void attach(const QVariant &,
                                   const QVector<AttributeItem> & = {},
                                   bool = true) const
        { throw LogicError(methodNotImplementedMessage("attach")); }
        /*! Attach a model to the parent. */
        inline virtual void attach(const Related &,
                                   const QVector<AttributeItem> & = {},
                                   bool = true) const
        { throw LogicError(methodNotImplementedMessage("attach")); }
        /*! Attach models to the parent. */
        inline virtual void attach(const std::map<typename BaseModel<Related>::KeyType,
                                                  QVector<AttributeItem>> &,
                                   bool = true) const
        { throw LogicError(methodNotImplementedMessage("attach")); }

        /*! Detach models from the relationship. */
        inline virtual int detach(const QVector<QVariant> &, bool = true) const
        { throw LogicError(methodNotImplementedMessage("detach")); }
        /*! Detach models from the relationship. */
        inline virtual int detach(bool = true) const
        { throw LogicError(methodNotImplementedMessage("detach")); }
        /*! Detach models from the relationship. */
        inline virtual int detach(const QVector<std::reference_wrapper<Related>> &,
                                  bool = true) const
        { throw LogicError(methodNotImplementedMessage("detach")); }
        /*! Detach model from the relationship. */
        inline virtual int detach(const QVariant &, bool = true) const
        { throw LogicError(methodNotImplementedMessage("detach")); }
        /*! Detach model from the relationship. */
        inline virtual int detach(const Related &, bool = true) const
        { throw LogicError(methodNotImplementedMessage("detach")); }

        /*! Sync the intermediate tables with a list of IDs. */
        inline virtual SyncChanges sync(
                const std::map<typename BaseModel<Related>::KeyType,
                               QVector<AttributeItem>> &,
                bool = true) const
        { throw LogicError(methodNotImplementedMessage("sync")); }
        /*! Sync the intermediate tables with a vector of IDs. */
        inline virtual SyncChanges sync(const QVector<QVariant> &, bool = true) const
        { throw LogicError(methodNotImplementedMessage("sync")); }

        /*! Sync the intermediate tables with a vector of IDs without detaching. */
        inline virtual SyncChanges syncWithoutDetaching(
                const std::map<typename BaseModel<Related>::KeyType,
                               QVector<AttributeItem>> &) const
        { throw LogicError(methodNotImplementedMessage("syncWithoutDetaching")); }
        /*! Sync the intermediate tables with a vector of IDs without detaching. */
        inline virtual SyncChanges syncWithoutDetaching(const QVector<QVariant> &) const
        { throw LogicError(methodNotImplementedMessage("syncWithoutDetaching")); }

        /*! Update an existing pivot record on the table. */
        inline virtual int updateExistingPivot(
                const QVariant &, QVector<AttributeItem>, bool = true) const
        { throw LogicError(methodNotImplementedMessage("updateExistingPivot")); }

    protected:
        /*! Initialize a Relation instance. */
        inline void init() const
        { addConstraints(); }

        /*! Get all of the primary keys for the vector of models. */
        QVector<QVariant>
        getKeys(const QVector<Model> &models, const QString &key = "") const;

        /*! The textual representation of the Relation type. */
        virtual QString relationTypeName() const = 0;

        /* During eager load, we secure m_parent to not become a dangling reference in
           TinyBuilder::eagerLoadRelation() by help of the dummyModel local variable.
           It has to be the reference, because eg BelongsTo::associate() directly
           modifies attributes of m_parent. */
        /*! The parent model instance. */
        Model &m_parent;
        /*! The related model instance. */
        const std::unique_ptr<Related> m_related;
        // TODO next would be good to use TinyBuilder alias instead of Builder silverqx
        /*! The Eloquent query builder instance. */
        std::unique_ptr<Builder<Related>> m_query;
        /*! Indicates if the relation is adding constraints. */
        static bool constraints;

    private:
        /*! Message for LogicError, when a user tries to call an unimplemented method. */
        inline QString methodNotImplementedMessage(const QString &method) const
        { return QStringLiteral("The '%1' method is not implemented "
                                "for the '%2' relation type.")
                    .arg(method, relationTypeName()); }
    };

    /*! The tag for one type relation. */
    class OneRelation
    {};

    /*! The tag for many type relation. */
    class ManyRelation
    {};

    /*! The tag for the relation which contains pivot table, like many-to-many. */
    class PivotRelation
    {};

    template<class Model, class Related>
    bool Relation<Model, Related>::constraints = true;

    template<class Model, class Related>
    Relation<Model, Related>::Relation(std::unique_ptr<Related> &&related, Model &parent)
        : m_parent(parent)
        , m_related(std::move(related))
        , m_query(m_related->newQuery())
    {}

    template<class Model, class Related>
    std::unique_ptr<Relation<Model, Related>>
    Relation<Model, Related>::noConstraints(
            const std::function<std::unique_ptr<Relation<Model, Related>>()> &callback)
    {
        const auto previous = constraints;

        constraints = false;
        auto relation = std::invoke(callback);
        constraints = previous;

        return relation;
    }

    template<class Model, class Related>
    void Relation<Model, Related>::touch() const
    {
        const auto &model = getRelated();

        if (!model.isIgnoringTouch())
            rawUpdate({
                {model.getUpdatedAtColumn(), model.freshTimestampString()}
            });
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::rawUpdate(const QVector<UpdateItem> &values) const
    {
        return m_query->update(values);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::find(const QVariant &id, const QStringList &columns) const
    {
        return m_query->find(id, columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::findOrNew(const QVariant &id,
                                        const QStringList &columns) const
    {
        return m_query->findOrNew(id, columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::findOrFail(const QVariant &id,
                                         const QStringList &columns) const
    {
        return m_query->findOrFail(id, columns);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::firstOrNew(const QVector<WhereItem> &attributes,
                                         const QVector<AttributeItem> &values) const
    {
        return m_query->firstOrNew(attributes, values);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::firstOrCreate(const QVector<WhereItem> &attributes,
                                            const QVector<AttributeItem> &values) const
    {
        return m_query->firstOrCreate(attributes, values);
    }

    template<class Model, class Related>
    Related Relation<Model, Related>::firstOrFail(const QStringList &columns) const
    {
        return m_query->firstOrFail(columns);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::firstWhere(
            const QString &column, const QString &comparison,
            const QVariant &value, const QString &condition) const
    {
        return m_query->firstWhere(column, comparison, value, condition);
    }

    template<class Model, class Related>
    std::optional<Related>
    Relation<Model, Related>::firstWhereEq(const QString &column, const QVariant &value,
                                           const QString &condition) const
    {
        return m_query->firstWhereEq(column, value, condition);
    }

    template<class Model, class Related>
    QVariant Relation<Model, Related>::value(const QString &column) const
    {
        return m_query->value(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::with(const QVector<WithItem> &relations) const
    {
        return m_query->with(relations);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::with(const QString &relation) const
    {
        return m_query->with(relation);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::without(const QVector<QString> &relations) const
    {
        return m_query->without(relations);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::without(const QString &relation) const
    {
        return m_query->without(relation);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::create(const QVector<AttributeItem> &attributes) const
    {
        return m_query->create(attributes);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::create(QVector<AttributeItem> &&attributes) const
    {
        return m_query->create(std::move(attributes));
    }

    template<class Model, class Related>
    std::optional<Model>
    Relation<Model, Related>::first(const QStringList &columns) const
    {
        return m_query->first(columns);
    }

    template<class Model, class Related>
    std::tuple<bool, std::optional<QSqlQuery>>
    Relation<Model, Related>::insert(const QVector<AttributeItem> &attributes) const
    {
        return m_query->insert(attributes);
    }

    // FEATURE dilemma primarykey, Model::KeyType vs QVariant silverqx
    template<class Model, class Related>
    quint64
    Relation<Model, Related>::insertGetId(const QVector<AttributeItem> &attributes) const
    {
        return m_query->insertGetId(attributes);
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::update(const QVector<UpdateItem> &values) const
    {
        return m_query->update(values);
    }

    template<class Model, class Related>
    Related
    Relation<Model, Related>::updateOrCreate(const QVector<WhereItem> &attributes,
                                             const QVector<AttributeItem> &values) const
    {
        return m_query->updateOrCreate(attributes, values);
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::remove() const
    {
        return m_query->remove();
    }

    template<class Model, class Related>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::deleteModels() const
    {
        return m_query->deleteModels();
    }

    template<class Model, class Related>
    void Relation<Model, Related>::truncate() const
    {
        m_query->truncate();
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::select(const QStringList columns) const
    {
        return m_query->select(columns);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::select(const QString column) const
    {
        return m_query->select(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::addSelect(const QStringList &columns) const
    {
        return m_query->addSelect(columns);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::addSelect(const QString &column) const
    {
        return m_query->addSelect(column);
    }

    template<class Model, class Related>
    Builder<Related> &Relation<Model, Related>::distinct() const
    {
        return m_query->distinct();
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::join(const QString &table, const QString &first,
                                   const QString &comparison, const QString &second,
                                   const QString &type, const bool where) const
    {
        return m_query->join(table, first, comparison, second, type, where);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::join(const QString &table,
                                   const std::function<void (JoinClause &)> &callback,
                                   const QString &type) const
    {
        return m_query->join(table, callback, type);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::joinWhere(const QString &table, const QString &first,
                                        const QString &comparison, const QString &second,
                                        const QString &type) const
    {
        return m_query->joinWhere(table, first, comparison, second, type);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::leftJoin(
            const QString &table, const QString &first,
            const QString &comparison, const QString &second) const
    {
        return m_query->leftJoin(table, first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::leftJoin(
            const QString &table,
            const std::function<void (JoinClause &)> &callback) const
    {
        return m_query->leftJoin(table, callback);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::leftJoinWhere(
            const QString &table, const QString &first,
            const QString &comparison, const QString &second) const
    {
        return m_query->leftJoinWhere(table, first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::rightJoin(
            const QString &table, const QString &first,
            const QString &comparison, const QString &second) const
    {
        return m_query->rightJoin(table, first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::rightJoin(
            const QString &table,
            const std::function<void (JoinClause &)> &callback) const
    {
        return m_query->rightJoin(table, callback);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::rightJoinWhere(
            const QString &table, const QString &first,
            const QString &comparison, const QString &second) const
    {
        return m_query->rightJoinWhere(table, first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::crossJoin(
            const QString &table, const QString &first,
            const QString &comparison, const QString &second) const
    {
        return m_query->crossJoin(table, first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::crossJoin(
            const QString &table,
            const std::function<void (JoinClause &)> &callback) const
    {
        return m_query->crossJoin(table, callback);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::where(const QString &column, const QString &comparison,
                                    const QVariant &value, const QString &condition) const
    {
        return m_query->where(column, comparison, value, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhere(const QString &column, const QString &comparison,
                                      const QVariant &value) const
    {
        return m_query->orWhere(column, comparison, value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereEq(const QString &column, const QVariant &value,
                                      const QString &condition) const
    {
        return m_query->whereEq(column, value, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereEq(const QString &column,
                                        const QVariant &value) const
    {
        return m_query->orWhereEq(column, value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::where(
            const std::function<void(Builder<Related> &)> &callback,
            const QString &condition) const
    {
        return m_query->where(callback, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhere(
            const std::function<void(Builder<Related> &)> &callback) const
    {
        return m_query->orWhere(callback);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::where(const QVector<WhereItem> &values,
                                    const QString &condition) const
    {
        return m_query->where(values, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhere(const QVector<WhereItem> &values) const
    {
        return m_query->orWhere(values);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereColumn(
            const QVector<WhereColumnItem> &values, const QString &condition) const
    {
        return m_query->whereColumn(values, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereColumn(const QVector<WhereColumnItem> &values) const
    {
        return m_query->orWhereColumn(values);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereColumn(
            const QString &first, const QString &comparison,
            const QString &second, const QString &condition) const
    {
        return m_query->whereColumn(first, comparison, second, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereColumn(
            const QString &first, const QString &comparison, const QString &second) const
    {
        return m_query->orWhereColumn(first, comparison, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereColumnEq(
            const QString &first, const QString &second, const QString &condition) const
    {
        return m_query->whereColumnEq(first, second, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereColumnEq(const QString &first,
                                              const QString &second) const
    {
        return m_query->orWhereColumnEq(first, second);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereIn(
            const QString &column, const QVector<QVariant> &values,
            const QString &condition, const bool nope) const
    {
        return m_query->whereIn(column, values, condition, nope);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereIn(const QString &column,
                                        const QVector<QVariant> &values) const
    {
        return m_query->orWhereIn(column, values);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereNotIn(
            const QString &column, const QVector<QVariant> &values,
            const QString &condition) const
    {
        return m_query->whereNotIn(column, values, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereNotIn(const QString &column,
                                           const QVector<QVariant> &values) const
    {
        return m_query->orWhereNotIn(column, values);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereNull(
            const QStringList &columns, const QString &condition, const bool nope) const
    {
        return m_query->whereNull(columns, condition, nope);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereNull(
            const QString &column, const QString &condition, const bool nope) const
    {
        return m_query->whereNull(column, condition, nope);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereNull(const QStringList &columns) const
    {
        return m_query->orWhereNull(columns);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereNull(const QString &column) const
    {
        return m_query->orWhereNull(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereNotNull(const QStringList &columns,
                                           const QString &condition) const
    {
        return m_query->whereNotNull(columns, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereNotNull(const QString &column,
                                           const QString &condition) const
    {
        return m_query->whereNotNull(column, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereNotNull(const QStringList &columns) const
    {
        return m_query->orWhereNotNull(columns);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orWhereNotNull(const QString &column) const
    {
        return m_query->orWhereNotNull(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::groupBy(const QStringList &groups) const
    {
        return m_query->groupBy(groups);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::groupBy(const QString &group) const
    {
        return m_query->groupBy(group);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::having(
            const QString &column, const QString &comparison,
            const QVariant &value, const QString &condition) const
    {
        return m_query->having(column, comparison, value, condition);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orHaving(const QString &column, const QString &comparison,
                                       const QVariant &value) const
    {
        return m_query->orHaving(column, comparison, value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orderBy(const QString &column,
                                      const QString &direction) const
    {
        return m_query->orderBy(column, direction);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::orderByDesc(const QString &column) const
    {
        return m_query->orderByDesc(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::latest(const QString &column) const
    {
        return m_query->latest(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::oldest(const QString &column) const
    {
        return m_query->oldest(column);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::reorder() const
    {
        return m_query->reorder();
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::reorder(const QString &column,
                                      const QString &direction) const
    {
        return m_query->reorder(column, direction);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::limit(const int value) const
    {
        return m_query->limit(value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::take(const int value) const
    {
        return m_query->take(value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::offset(const int value) const
    {
        return m_query->offset(value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::skip(const int value) const
    {
        return m_query->skip(value);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::forPage(const int page, const int perPage) const
    {
        return m_query->forPage(page, perPage);
    }

    template<class Model, class Related>
    template<typename T> requires std::is_arithmetic_v<T>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::increment(const QString &column, const T amount,
                                        const QVector<UpdateItem> &extra) const
    {
        return m_query->increment(column, amount, extra);
    }

    template<class Model, class Related>
    template<typename T> requires std::is_arithmetic_v<T>
    std::tuple<int, QSqlQuery>
    Relation<Model, Related>::decrement(const QString &column, const T amount,
                                        const QVector<UpdateItem> &extra) const
    {
        return m_query->decrement(column, amount, extra);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereKey(const QVariant &id) const
    {
        return m_query->whereKey(id);
    }

    template<class Model, class Related>
    Builder<Related> &
    Relation<Model, Related>::whereKeyNot(const QVariant &id) const
    {
        return m_query->whereKeyNot(id);
    }

    template<class Model, class Related>
    QVector<QVariant>
    Relation<Model, Related>::getKeys(const QVector<Model> &models,
                                      const QString &key) const
    {
        QVector<QVariant> keys;

        // BUG what if key is empty/null/invalid, looks orig. implementation, there is used "->values()->unique(null, true)" silverqx
        for (const auto &model : models)
            keys.append(key.isEmpty() ? model.getKey()
                                      : model.getAttribute(key));

        using namespace ranges;
        return keys |= actions::sort(less {}, &QVariant::value<typename Model::KeyType>)
                       | actions::unique;
    }

} // namespace Orm::Tiny::Relations
} // namespace Orm::Tiny
} // namespace Orm
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif

#endif // RELATION_H
