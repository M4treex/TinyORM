#pragma once
#ifndef TOM_COMMANDS_MAKE_MODELCOMMAND_HPP
#define TOM_COMMANDS_MAKE_MODELCOMMAND_HPP

#include <orm/macros/systemheader.hpp>
TINY_SYSTEM_HEADER

#include <unordered_set>

#include "tom/commands/command.hpp"
#include "tom/commands/make/support/modelcreator.hpp"
#include "tom/tomconstants.hpp"

TINYORM_BEGIN_COMMON_NAMESPACE

namespace Tom::Commands::Make
{

    /*! Create a new model class. */
    class ModelCommand : public Command
    {
        Q_DISABLE_COPY(ModelCommand)

        /*! Alias for the filesystem path. */
        using fspath = std::filesystem::path;
        /*! Alias for the command line option values. */
        using CmdOptions = Support::ModelCreator::CmdOptions;
        /*! Alias for the foreign keys. */
        using ForeignKeys = Support::ModelCreator::ForeignKeys;

    public:
        /*! Constructor. */
        ModelCommand(Application &application, QCommandLineParser &parser);
        /*! Virtual destructor. */
        inline ~ModelCommand() override = default;

        /*! The console command name. */
        inline QString name() const override;
        /*! The console command description. */
        inline QString description() const override;

        /*! The console command positional arguments signature. */
        const std::vector<PositionalArgument> &positionalArguments() const override;
        /*! The signature of the console command. */
        QList<QCommandLineOption> optionsSignature() const override;

        /*! Execute the console command. */
        int run() override;

    protected:
        /*! Prepare a model class names. */
        static std::tuple<QString, CmdOptions>
        prepareModelClassnames(QString &&className, CmdOptions &&cmdOptions);

        /*! Show unused options warning. */
        void showUnusedOptionsWarnings(const CmdOptions &cmdOptions);

        /*! Write the model file to the disk. */
        void writeModel(const QString &className, const CmdOptions &cmdOptions);

        /*! Create command line options instance. */
        CmdOptions createCmdOptions();

        /* Foreign key names */
        /*! Divide foreign key names by relation types. */
        ForeignKeys prepareForeignKeys(const QStringList &foreignKeyValues);
        /*! Try to start a new relation during foreign key names search. */
        static bool startNewRelation(
                    const std::unordered_set<QString> &relationNames, QString &option,
                    QString &currentRelation, ForeignKeys &foreignKeys,
                    bool &wasForeignKeySet, bool &wasForeignKeySetPartial);
        /*! Insert the default value if no foreign key was passed on the cmd. line. */
        static void insertEmptyForeignList(const QString &currentRelation,
                                           ForeignKeys &foreignKeys);
        /*! Foreign key name found, assign it to the correct relation type. */
        static void insertForeignKeyName(
                    const QString &currentRelation, ForeignKeys &foreignKeys,
                    const QStringList &foreignKeyValues,
                    QStringList::size_type &foreignIndex, bool &wasForeignKeySet,
                    bool &wasForeignKeySetPartial);
        /*! Foreign key name found, assign it to the correct relation type (for btm). */
        static void insertForeignKeyNameBtm(
                    ForeignKeys &foreignKeys, const QStringList &foreignKeyValues,
                    QStringList::size_type &foreignIndex, bool &wasForeignKeySet,
                    bool &wasForeignKeySetPartial);
        /*! Show unused foreign key option warning. */
        void showUnusedForeignKeyWarning();

        /* Others */
        /*! Get the model path (either specified by the --path option or the default
            location). */
        fspath getModelPath() const;

        /*! The model creator instance. */
        Support::ModelCreator m_creator {};
        /*! Indicates whether the unused warning have been shown. */
        bool m_shownUnusedWarning = false;

    private:
        /*! Throw if the model name constains a namespace or path. */
        static void throwIfContainsNamespaceOrPath(const QString &className,
                                                   const QString &source);
    };

    /* public */

    QString ModelCommand::name() const
    {
        return Constants::MakeModel;
    }

    QString ModelCommand::description() const
    {
        return QStringLiteral("Create a new model class");
    }

} // namespace Tom::Commands::Make

TINYORM_END_COMMON_NAMESPACE

#endif // TOM_COMMANDS_MAKE_MODELCOMMAND_HPP