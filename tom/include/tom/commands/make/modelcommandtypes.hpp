#pragma once
#ifndef TOM_COMMANDS_MAKE_MODELCOMMANDTYPES_HPP
#define TOM_COMMANDS_MAKE_MODELCOMMANDTYPES_HPP

#include <orm/macros/systemheader.hpp>
TINY_SYSTEM_HEADER

#include <QStringList>

#include <orm/macros/commonnamespace.hpp>

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Make
{

    /*! Foreign key names for the belongs-to-many relation. */
    struct BelongToManyForeignKeys
    {
        /*! The foreign key of the parent model. */
        QString foreignPivotKey;
        /*! The associated key of the relation. */
        QString relatedPivotKey;
    };

    /*! Foreign keys lists divided by relation types. */
    struct ForeignKeys
    {
        /*! Foreign keys for one-to-one relations. */
        QStringList oneToOne {};
        /*! Foreign keys for one-to-many relations. */
        QStringList oneToMany {};
        /*! Foreign keys for belongs-to relations. */
        QStringList belongsTo {};
        /*! Foreign keys for belongs-to-many relations. */
        std::vector<BelongToManyForeignKeys> belongsToMany {};
    };

    /*! Struct to hold command line option values. */
    struct CmdOptions
    {
        /* Relationship methods */
        /*! Related class names for the one-to-one relationship. */
        QStringList oneToOneList;
        /*! Related class names for the one-to-many relationship. */
        QStringList oneToManyList;
        /*! Related class names for the belongs-to relationship. */
        QStringList belongsToList;
        /*! Related class name for the belongs-to-many relationship. */
        QStringList belongsToManyList;
        /* Common for all relationship methods */
        /*! The foreign key names list, every relation can have one foreign key. */
        ForeignKeys foreignKeys;
        /* Belongs-to-many related */
        /*! The pivot table name. */
        QStringList pivotTables;
        /*! The class name of the pivot class for the belongs-to-many relationship. */
        QStringList pivotClasses;
        /*! The name for the pivot relation. */
        QStringList asList;
        /*! Extra attributes for the pivot model. */
        std::vector<QStringList> withPivotList;
        /*! Pivot table with timestamps. */
        std::vector<bool> withTimestampsList;
        /* Model related */
        /*! The connection name for the model. */
        QString connection;
        /*! The table associated with the model. */
        QString table;
        /*! Disable timestamping of the model. */
        bool disableTimestamps;
    };

} // namespace Tom::Commands::Make

TINYORM_END_COMMON_NAMESPACE

#endif // TOM_COMMANDS_MAKE_MODELCOMMANDTYPES_HPP
