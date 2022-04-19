#pragma once
#ifndef TOM_COMMANDS_INSPIRECOMMAND_HPP
#define TOM_COMMANDS_INSPIRECOMMAND_HPP

#include <orm/macros/systemheader.hpp>
TINY_SYSTEM_HEADER

#include "tom/commands/command.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands
{

    /*! Display an inspiring quote. */
    class InspireCommand : public Command
    {
        Q_DISABLE_COPY(InspireCommand)

    public:
        /*! Constructor. */
        InspireCommand(Application &application, QCommandLineParser &parser);
        /*! Virtual destructor. */
        inline ~InspireCommand() override = default;

        /*! The console command name. */
        inline QString name() const override;
        /*! The console command description. */
        inline QString description() const override;

        /*! Execute the console command. */
        int run() override;
    };

    /* public */

    QString InspireCommand::name() const
    {
        return QStringLiteral("inspire");
    }

    QString InspireCommand::description() const
    {
        return QLatin1String("Display an inspiring quote");
    }

} // namespace Tom::Commands

TINYORM_END_COMMON_NAMESPACE

#endif // TOM_COMMANDS_INSPIRECOMMAND_HPP
