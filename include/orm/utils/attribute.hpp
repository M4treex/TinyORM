#pragma once
#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include "orm/macros/systemheader.hpp"
TINY_SYSTEM_HEADER

#include "orm/ormtypes.hpp"
#include "orm/macros/export.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm::Utils::Attribute
{
    /*! Convert a AttributeItem QVector to QVariantMap. */
    SHAREDLIB_EXPORT QVariantMap
    convertVectorToMap(const QVector<AttributeItem> &attributes);
    /*! Convert a vector of AttributeItem QVectors to the vector of QVariantMaps. */
    SHAREDLIB_EXPORT QVector<QVariantMap>
    convertVectorsToMaps(const QVector<QVector<AttributeItem>> &attributesVector);

    /*! Convert a AttributeItem QVector to UpdateItem QVector. */
    SHAREDLIB_EXPORT QVector<UpdateItem>
    convertVectorToUpdateItem(const QVector<AttributeItem> &attributes);
    /*! Convert a AttributeItem QVector to UpdateItem QVector. */
    SHAREDLIB_EXPORT QVector<UpdateItem>
    convertVectorToUpdateItem(QVector<AttributeItem> &&attributes);

    /*! Remove attributes which have duplicite keys and leave only the last one. */
    SHAREDLIB_EXPORT QVector<AttributeItem>
    removeDuplicitKeys(const QVector<AttributeItem> &attributes);

    /*! Join attributes and values for firstOrXx methods. */
    SHAREDLIB_EXPORT QVector<AttributeItem>
    joinAttributesForFirstOr(const QVector<WhereItem> &attributes,
                             const QVector<AttributeItem> &values,
                             const QString &keyName);
} // namespace Orm::Utils::Attribute
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif

#endif // ATTRIBUTE_HPP
