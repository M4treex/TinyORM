#include "orm/schema/grammars/mysqlgrammar.hpp"

#ifdef TINYORM_COMMON_NAMESPACE
namespace TINYORM_COMMON_NAMESPACE
{
#endif
namespace Orm::Schema::Grammars
{

QString MySqlGrammar::compileColumnListing(const QString &) const
{
    return "select `column_name` from `information_schema`.`columns` "
           "where `table_schema` = ? and `table_name` = ?";
}

} // namespace Orm::Schema::Grammars
#ifdef TINYORM_COMMON_NAMESPACE
} // namespace TINYORM_COMMON_NAMESPACE
#endif
