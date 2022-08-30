#pragma once
#ifndef ORM_TINY_RELATIONS_BASEPIVOT_HPP
#define ORM_TINY_RELATIONS_BASEPIVOT_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/tiny/model.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Orm::Tiny::Relations
{

    /*! Tag for the Pivot model. */
    class IsPivotModel
    {};

    /*! Base class for Pivot models. */
    template<typename PivotModel>
    class BasePivot : public Model<PivotModel>,
                      public IsPivotModel
    {
        friend Model<PivotModel>;

        /*! Alias for the string utils. */
        using StringUtils = Orm::Tiny::Utils::String;
        /*! Alias for the type utils. */
        using TypeUtils = Orm::Utils::Type;

    public:
        /*! Inherit constructors. */
        using Model<PivotModel>::Model;

        /* AsPivot */
        /*! Create a new pivot model instance. */
        template<typename Parent>
        static PivotModel
        fromAttributes(const Parent &parent, const QVector<AttributeItem> &attributes,
                       const QString &table, bool exists = false);
        template<typename Parent>
        /*! Create a new pivot model from raw values returned from a query. */
        static PivotModel
        fromRawAttributes(const Parent &parent, const QVector<AttributeItem> &attributes,
                          const QString &table, bool exists = false);

        /*! Set the key names for the pivot model instance. */
        PivotModel &setPivotKeys(const QString &foreignKey, const QString &relatedKey);
        /*! Determine if the pivot model or given attributes has timestamp attributes. */
        bool hasTimestampAttributes(const QVector<AttributeItem> &attributes) const;
        /*! Determine if the pivot model or given attributes has timestamp attributes. */
        bool hasTimestampAttributes() const;

        /* Hide methods from a Model<PivotModel> */
        /* Different implementation than the Model and instead of make them virtual
           I used the CRTP pattern in the Model to properly call them, I can not use
           virtual here, because of the Virtual Friend Function Idiom, it would
           be very disarranged. */
        /*! Delete the pivot model record from the database. */
        bool remove();
        /*! Delete the pivot model record from the database (alias). */
        inline bool deleteModel();

        /*! Get the table associated with the model. */
        QString getTable() const;
        /*! Get the foreign key column name. */
        const QString &getForeignKey() const noexcept;
        /*!  Get the "related key" column name. */
        const QString &getRelatedKey() const noexcept;

        // TODO fuckup, timestamps in pivot, the solution is to set CREATED_AT and UPDATED_AT right away in the fromAttributes method when I still have access to the parent, then I won't have to save a pointer to the parent. I can still save pointer to parent, but not for obtaining this timestamp column names. old - I will solve it when I will have to use timestamps in the code, anyway may be I will not need it, because I can pass to the method right away what I will need silverqx
        // TODO also don't forget unsetRelations() if pivotParent will be implemented silverqx
        /*! The parent model of the relationship. */
//        template<typename Parent>
//        inline static const Parent *pivotParent = nullptr;

    protected:
        /* AsPivot */
        /*! Set the keys for a save update query. */
        TinyBuilder<PivotModel> &
        setKeysForSaveQuery(TinyBuilder<PivotModel> &query);
        /*! Set the keys for a select query. */
        TinyBuilder<PivotModel> &
        setKeysForSelectQuery(TinyBuilder<PivotModel> &query);

        /*! Get the query builder for a delete operation on the pivot. */
        std::unique_ptr<TinyBuilder<PivotModel>> getDeleteQuery();

        /* BasePivot */
        /*! Indicates if the ID is auto-incrementing. */
        bool u_incrementing = false;
        /*! The attributes that aren't mass assignable. */
        T_THREAD_LOCAL
        inline static QStringList u_guarded;

        /* AsPivot */
        /*! The name of the foreign key column. */
        QString m_foreignKey;
        /*! The name of the "other key" column. */
        QString m_relatedKey;
    };

    /* public */

    /* AsPivot */

    template<typename PivotModel>
    template<typename Parent>
    PivotModel
    BasePivot<PivotModel>::fromAttributes(
            const Parent &parent, const QVector<AttributeItem> &attributes,
            const QString &table, const bool exists)
    {
        PivotModel instance;

        instance.setUseTimestamps(instance.hasTimestampAttributes(attributes));

        /* The pivot model is a "dynamic" model since we will set the tables dynamically
           for the instance. This allows it work for any intermediate tables for the
           many to many relationship that are defined by this developer's classes. */
        instance.setConnection(parent.getConnectionName())
            .setTable(table)
            .forceFill(attributes)
            .syncOriginal();

        /* We store off the parent instance so we will access the timestamp column names
           for the model, since the pivot model timestamps aren't easily configurable
           from the developer's point of view. We can use the parents to get these. */
//        pivotParent<Parent> = &parent;

        instance.exists = exists;

        return instance;
    }

    template<typename PivotModel>
    template<typename Parent>
    PivotModel
    BasePivot<PivotModel>::fromRawAttributes(
            const Parent &parent, const QVector<AttributeItem> &attributes,
            const QString &table, const bool exists)
    {
        auto instance = fromAttributes(parent, {}, table, exists);

        instance.setUseTimestamps(instance.hasTimestampAttributes(attributes));

        instance.setRawAttributes(attributes, exists);

        return instance;
    }

    template<typename PivotModel>
    PivotModel &
    BasePivot<PivotModel>::setPivotKeys(const QString &foreignKey,
                                        const QString &relatedKey)
    {
        m_foreignKey = foreignKey;

        m_relatedKey = relatedKey;

        return this->model();
    }

    template<typename PivotModel>
    bool BasePivot<PivotModel>::hasTimestampAttributes(
            const QVector<AttributeItem> &attributes) const
    {
        const auto &createdAtColumn = this->getCreatedAtColumn();

        for (const auto &attribute : attributes)
            if (attribute.key == createdAtColumn)
                return true;

        return false;
    }

    template<typename PivotModel>
    bool BasePivot<PivotModel>::hasTimestampAttributes() const
    {
        return hasTimestampAttributes(this->m_attributes);
    }

    /* Hide methods from a Model<PivotModel> */

    // NOTE api different silverqx
    template<typename PivotModel>
    bool BasePivot<PivotModel>::remove()
    {
        /* If a primary key is defined on the current Pivot model, we can use
           Model's 'remove' method, otherwise we have to build a query with
           the help of QueryBuilder's 'where' method. */
        if (this->m_attributesHash.contains(this->getKeyName()))
            return Model<PivotModel>::remove();

        // FEATURE events silverqx
//        if (fireModelEvent("deleting") == false)
//            return false;

        this->touchOwners();

        // Ownership of a unique_ptr()
        int affected = 0;
        std::tie(affected, std::ignore) = getDeleteQuery()->remove();

        this->exists = false;

        // FEATURE events silverqx
//        fireModelEvent("deleted", false);

        return affected > 0;
    }

    template<typename PivotModel>
    bool BasePivot<PivotModel>::deleteModel()
    {
        return this->model().remove();
    }

    template<typename PivotModel>
    QString BasePivot<PivotModel>::getTable() const
    {
        const auto &table = this->model().u_table;

        // Get singularizes snake-case table name
        if (table.isEmpty())
            return StringUtils::singular(
                        StringUtils::snake(TypeUtils::classPureBasename<PivotModel>()));

        return table;
    }

    template<typename PivotModel>
    const QString &BasePivot<PivotModel>::getForeignKey() const noexcept
    {
        return m_foreignKey;
    }

    template<typename PivotModel>
    const QString &BasePivot<PivotModel>::getRelatedKey() const noexcept
    {
        return m_relatedKey;
    }

    /* protected */

    /* AsPivot */

    template<typename PivotModel>
    TinyBuilder<PivotModel> &
    BasePivot<PivotModel>::setKeysForSaveQuery(TinyBuilder<PivotModel> &query)
    {
        return this->model().setKeysForSelectQuery(query);
    }

    template<typename PivotModel>
    TinyBuilder<PivotModel> &
    BasePivot<PivotModel>::setKeysForSelectQuery(TinyBuilder<PivotModel> &query)
    {
        /* If the pivot table contains a primary key then use this primary key
           in the where clause. */
        if (const auto &primaryKeyName = this->getKeyName();
            this->m_attributesHash.contains(primaryKeyName)
        )
            // Also check if this primary key is valid
            if (const auto id = this->getKeyForSelectQuery();
                id.isValid() && !id.isNull()
            )
                return Model<PivotModel>::setKeysForSelectQuery(query);

        // NOTE api different, we are using parenthesis around the following keys, I think it's a little safer silverqx
        return query.where({
            {m_foreignKey, this->getOriginal(m_foreignKey,
                                             this->getAttribute(m_foreignKey))},
            {m_relatedKey, this->getOriginal(m_relatedKey,
                                             this->getAttribute(m_relatedKey))},
        });
    }

    template<typename PivotModel>
    std::unique_ptr<TinyBuilder<PivotModel>>
    BasePivot<PivotModel>::getDeleteQuery()
    {
        // Ownership of a unique_ptr()
        auto builder = this->newQueryWithoutRelationships();

        builder->where({
            {m_foreignKey, this->getOriginal(m_foreignKey,
                                             this->getAttribute(m_foreignKey))},
            {m_relatedKey, this->getOriginal(m_relatedKey,
                                             this->getAttribute(m_relatedKey))},
        });

        return builder;
    }

} // namespace Orm::Tiny::Relations

TINYORM_END_COMMON_NAMESPACE

#endif // ORM_TINY_RELATIONS_BASEPIVOT_HPP
