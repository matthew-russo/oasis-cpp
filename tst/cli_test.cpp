#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "interaction/cli/parser.hpp"

using namespace oasis::interaction::cli;

// ============================= Definition Tests ==============================

TEST(CliParserTest, ArgDefinitionWithNoNameReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliDefinitionError > maybeArgDef = builder
    .withHelp("test help msg")
    ->withType(CliType::Bool)
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliDefinitionError::DefinitionMissingName);
}

TEST(CliParserTest, ArgDefinitionWithNoHelpReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliDefinitionError > maybeArgDef = builder
    .withLongName("name")
    ->withType(CliType::Bool)
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliDefinitionError::DefinitionMissingHelpMessage);
}

TEST(CliParserTest, ArgDefinitionWithNoTypeReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliDefinitionError > maybeArgDef = builder
    .withLongName("name")
    ->withHelp("test help msg")
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliDefinitionError::DefinitionMissingType);
}

TEST(CliParserTest, SuccessfulArgDefinition) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliDefinitionError > maybeArgDef = builder
    .withLongName("name")
    ->withShortName("n")
    ->withHelp("test help msg")
    ->withType(CliType::Bool)
    ->isRequired(true)
    ->build();
  EXPECT_TRUE(maybeArgDef.has_value());
  ArgDefinition argDef = maybeArgDef.value();

  EXPECT_STREQ(argDef.getLongName(), "name");
  EXPECT_TRUE(argDef.getShortName().has_value());
  EXPECT_STREQ(argDef.getShortName().value(), "n");
  EXPECT_STREQ(argDef.getHelp(), "test help msg");
  EXPECT_EQ(argDef.getType(), CliType::Bool);
  EXPECT_TRUE(argDef.isRequired());
}

TEST(CliParserTest, CommandDefinitionWithNoNameReturnsError) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withHelp("test help msg")
    ->build();
  EXPECT_FALSE(maybeCommandDef.has_value());
  EXPECT_EQ(maybeCommandDef.error(), CliDefinitionError::DefinitionMissingName);
}

TEST(CliParserTest, CommandDefinitionWithNoHelpReturnsError) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withName("name")
    ->build();
  EXPECT_FALSE(maybeCommandDef.has_value());
  EXPECT_EQ(maybeCommandDef.error(), CliDefinitionError::DefinitionMissingHelpMessage);
}

TEST(CliParserTest, SuccessfulBasicCommandDefinition) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withName("name")
    ->withHelp("test help msg")
    ->build();
  EXPECT_TRUE(maybeCommandDef.has_value());
  CommandDefinition commandDef = maybeCommandDef.value();

  EXPECT_STREQ(commandDef.getName(), "name");
  EXPECT_STREQ(commandDef.getHelp(), "test help msg");
  EXPECT_TRUE(commandDef.getPossibleArgs().empty());
  EXPECT_TRUE(commandDef.getPossibleSubcommands().empty());
}

TEST(CliParserTest, SuccessfulCommandWithArgsDefinition) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withName("name")
    ->withHelp("test help msg")
    ->withArg(
      ArgDefinitionBuilder()
        .withLongName("commandArg1")
        ->withHelp("test help msg for commandArg1")
        ->withType(CliType::U64)
        ->build()
        .value()
    )
    ->withArg(
      ArgDefinitionBuilder()
        .withLongName("commandArg2")
        ->withHelp("test help msg for commandArg2")
        ->withType(CliType::I64)
        ->build()
        .value()
    )
    ->build();
  EXPECT_TRUE(maybeCommandDef.has_value());
  CommandDefinition commandDef = maybeCommandDef.value();

  EXPECT_STREQ(commandDef.getName(), "name");
  EXPECT_STREQ(commandDef.getHelp(), "test help msg");

  EXPECT_EQ(commandDef.getPossibleArgs().size(), 2);

  ArgDefinition arg1 = commandDef.getPossibleArgs()[0];
  EXPECT_STREQ(arg1.getLongName(), "commandArg1");
  EXPECT_STREQ(arg1.getHelp(), "test help msg for commandArg1");
  EXPECT_EQ(arg1.getType(), CliType::U64);

  ArgDefinition arg2 = commandDef.getPossibleArgs()[1];
  EXPECT_STREQ(arg2.getLongName(), "commandArg2");
  EXPECT_STREQ(arg2.getHelp(), "test help msg for commandArg2");
  EXPECT_EQ(arg2.getType(), CliType::I64);

  EXPECT_TRUE(commandDef.getPossibleSubcommands().empty());
}

TEST(CliParserTest, SuccessfulCommandWithSubcommandsDefinition) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withName("name")
    ->withHelp("test help msg")
    ->withSubcommand(
      CommandDefinitionBuilder()
        .withName("subcommand1")
        ->withHelp("test help msg for subcommand1")
        ->build()
        .value()
    )
    ->withSubcommand(
      CommandDefinitionBuilder()
        .withName("subcommand2")
        ->withHelp("test help msg for subcommand2")
        ->build()
        .value()
    )
    ->build();
  EXPECT_TRUE(maybeCommandDef.has_value());
  CommandDefinition commandDef = maybeCommandDef.value();

  EXPECT_STREQ(commandDef.getName(), "name");
  EXPECT_STREQ(commandDef.getHelp(), "test help msg");

  EXPECT_TRUE(commandDef.getPossibleArgs().empty());

  EXPECT_EQ(commandDef.getPossibleSubcommands().size(), 2);

  CommandDefinition subcommand1 = commandDef.getPossibleSubcommands()[0];
  EXPECT_STREQ(subcommand1.getName(), "subcommand1");
  EXPECT_STREQ(subcommand1.getHelp(), "test help msg for subcommand1");
  EXPECT_TRUE(subcommand1.getPossibleArgs().empty());
  EXPECT_TRUE(subcommand1.getPossibleSubcommands().empty());

  CommandDefinition subcommand2 = commandDef.getPossibleSubcommands()[1];
  EXPECT_STREQ(subcommand2.getName(), "subcommand2");
  EXPECT_STREQ(subcommand2.getHelp(), "test help msg for subcommand2");
  EXPECT_TRUE(subcommand2.getPossibleArgs().empty());
  EXPECT_TRUE(subcommand2.getPossibleSubcommands().empty());
}

TEST(CliParserTest, SuccessfulComplexCommand) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliDefinitionError > maybeCommandDef = builder
    .withName("name")
    ->withHelp("test help msg")
    ->withArg(
      ArgDefinitionBuilder()
        .withLongName("commandArg1")
        ->withHelp("test help msg for commandArg1")
        ->withType(CliType::U64)
        ->build()
        .value()
    )
    ->withSubcommand(
      CommandDefinitionBuilder()
        .withName("subcommand1")
        ->withHelp("test help msg for subcommand1")
        ->withArg(
          ArgDefinitionBuilder()
            .withLongName("subcommand1Arg1")
            ->withHelp("test help msg for subcommand1Arg1")
            ->withType(CliType::I64)
            ->build()
            .value()
        )
        ->build()
        .value()
    )
    ->withSubcommand(
      CommandDefinitionBuilder()
        .withName("subcommand2")
        ->withHelp("test help msg for subcommand2")
        ->withSubcommand(
          CommandDefinitionBuilder()
            .withName("subcommand2subcommand1")
            ->withHelp("test help msg for subcommand2subcommand1")
            ->build()
            .value()
        )
        ->build()
        .value()
    )
    ->build();
  EXPECT_TRUE(maybeCommandDef.has_value());
  CommandDefinition commandDef = maybeCommandDef.value();

  EXPECT_STREQ(commandDef.getName(), "name");
  EXPECT_STREQ(commandDef.getHelp(), "test help msg");

  EXPECT_EQ(commandDef.getPossibleArgs().size(), 1);
  ArgDefinition arg1 = commandDef.getPossibleArgs()[0];
  EXPECT_STREQ(arg1.getLongName(), "commandArg1");
  EXPECT_STREQ(arg1.getHelp(), "test help msg for commandArg1");
  EXPECT_EQ(arg1.getType(), CliType::U64);

  EXPECT_EQ(commandDef.getPossibleSubcommands().size(), 2);

  CommandDefinition subcommand1 = commandDef.getPossibleSubcommands()[0];
  EXPECT_STREQ(subcommand1.getName(), "subcommand1");
  EXPECT_STREQ(subcommand1.getHelp(), "test help msg for subcommand1");
  EXPECT_EQ(subcommand1.getPossibleArgs().size(), 1);
  ArgDefinition subcommand1Arg1 = subcommand1.getPossibleArgs()[0];
  EXPECT_STREQ(subcommand1Arg1.getLongName(), "subcommand1Arg1");
  EXPECT_STREQ(subcommand1Arg1.getHelp(), "test help msg for subcommand1Arg1");
  EXPECT_EQ(subcommand1Arg1.getType(), CliType::I64);
  EXPECT_TRUE(subcommand1.getPossibleSubcommands().empty());

  CommandDefinition subcommand2 = commandDef.getPossibleSubcommands()[1];
  EXPECT_STREQ(subcommand2.getName(), "subcommand2");
  EXPECT_STREQ(subcommand2.getHelp(), "test help msg for subcommand2");
  EXPECT_TRUE(subcommand2.getPossibleArgs().empty());
  EXPECT_EQ(subcommand2.getPossibleSubcommands().size(), 1);
  CommandDefinition subcommand2subcommand1 = subcommand2.getPossibleSubcommands()[0];
  EXPECT_STREQ(subcommand2subcommand1.getName(), "subcommand2subcommand1");
  EXPECT_STREQ(subcommand2subcommand1.getHelp(), "test help msg for subcommand2subcommand1");
  EXPECT_TRUE(subcommand2subcommand1.getPossibleArgs().empty());
  EXPECT_TRUE(subcommand2subcommand1.getPossibleSubcommands().empty());
}

TEST(CliParserTest, EmptyParserReturnsError) {
  ParserBuilder builder;
  std::expected< Parser, CliDefinitionError > maybeParser = builder.build();
  EXPECT_FALSE(maybeParser.has_value());
  EXPECT_EQ(maybeParser.error(), CliDefinitionError::AtLeastOneCommandRequired);
}

// ============================= ArgParser Tests ==============================

TEST(CliParserTest, ArgParserReturnsOkNoneWithEmptyArgs) {
  int offset(0);
  std::vector< const char* > cliArgs;
  std::vector< ArgDefinition > argDefs;

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_FALSE(maybeArg.value().has_value());
}

TEST(CliParserTest, ArgParserReturnsUnknownArgErrorWithEmptyArgDefs) {
  int offset(0);
  std::vector< const char* > cliArgs { "--file", "test.txt" };
  std::vector< ArgDefinition > argDefs;

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_FALSE(maybeArg.has_value());
  EXPECT_EQ(maybeArg.error(), CliParsingError::UnknownArgument);
}

TEST(CliParserTest, ArgParserReturnsOkNoneIfWordDoesntStartWithHyphen) {
  int offset(0);
  std::vector< const char* > cliArgs { "test.txt" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("file", "f", "test arg", CliType::String, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_FALSE(maybeArg.value().has_value());
}

TEST(CliParserTest, ArgParserParsesLongArg) {
  int offset(0);
  std::vector< const char* > cliArgs { "--file", "test.txt" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("file", std::nullopt, "test arg", CliType::String, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliLongName("file") });
  EXPECT_EQ(arg.getValue(), CliValue { std::string { "test.txt" } });
}

TEST(CliParserTest, ArgParserParsesShortArg) {
  int offset(0);
  std::vector< const char* > cliArgs { "-f", "test.txt" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("file", "f", "test arg", CliType::String, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliShortName("f") });
  EXPECT_EQ(arg.getValue(), CliValue { std::string { "test.txt" } });
}

TEST(CliParserTest, ArgParserParsesU64) {
  int offset(0);
  std::vector< const char* > cliArgs { "-n",  "42" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("num", "n", "test arg", CliType::U64, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliShortName("n") });
  EXPECT_EQ(arg.getValue(), CliValue { uint64_t(42) });
}

TEST(CliParserTest, ArgParserParsesI64) {
  int offset(0);
  std::vector< const char* > cliArgs { "-n", "-42" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("num", "n", "test arg", CliType::I64, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliShortName("n") });
  EXPECT_EQ(arg.getValue(), CliValue { int64_t(-42) });
}

TEST(CliParserTest, ArgParserParsesBoolTrue) {
  int offset(0);
  std::vector< const char* > cliArgs { "-b", "true" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("bool", "b", "test arg", CliType::Bool, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliShortName("b") });
  EXPECT_EQ(arg.getValue(), CliValue { bool(true) });
}

TEST(CliParserTest, ArgParserParsesBoolFalse) {
  int offset(0);
  std::vector< const char* > cliArgs { "-b", "false" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("bool", "b", "test arg", CliType::Bool, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliShortName("b") });
  EXPECT_EQ(arg.getValue(), CliValue { bool(false) });
}

TEST(CliParserTest, ArgParserFailsToParseInvalidBool) {
  int offset(0);
  std::vector< const char* > cliArgs { "-b", "not-a-bool" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("bool", "b", "test arg", CliType::Bool, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_FALSE(maybeArg.has_value());
  EXPECT_EQ(maybeArg.error(), CliParsingError::InvalidBooleanValue);
}

TEST(CliParserTest, ArgParserParsesString) {
  int offset(0);
  std::vector< const char* > cliArgs { "--file", "test.txt" };
  std::vector< ArgDefinition > argDefs { ArgDefinition("file", std::nullopt, "test arg", CliType::String, true) };

  ArgParser argParser(&offset, std::span { cliArgs }, std::span { argDefs });
  std::expected< std::optional< Arg >, CliParsingError> maybeArg = argParser.parse();

  EXPECT_TRUE(maybeArg.has_value());
  EXPECT_TRUE(maybeArg.value().has_value());

  Arg arg = maybeArg.value().value();

  EXPECT_EQ(arg.getName(), CliArgName { CliLongName("file") });
  EXPECT_EQ(arg.getValue(), CliValue { std::string { "test.txt" } });
}

// ============================= CommandParser Tests ==============================

TEST(CliParserTest, CommandParserReturnsOkNoneWithEmptyArgs) {
  int offset(0);
  std::vector< const char* > cliArgs;
  std::vector< CommandDefinition > commandDefs;

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_FALSE(maybeCommand.value().has_value());
}

TEST(CliParserTest, CommandParserReturnsOkNoneWithEmptyCommandDef) {
  int offset(0);
  std::vector< const char* > cliArgs { "command" };
  std::vector< CommandDefinition > commandDefs;

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_FALSE(maybeCommand.value().has_value());
}

TEST(CliParserTest, CommandParserParsesBasicCommandWithNoArgsOrSubcommands) {
  int offset(0);
  std::vector< const char* > cliArgs { "command" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition("command", "test command", {}, {})
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_TRUE(command.getArgs().empty());
  EXPECT_FALSE(command.getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserParsesCommandWithSingleArg) {
  int offset(0);
  std::vector< const char* > cliArgs { "command", "--file", "test.txt" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      { ArgDefinition("file", std::nullopt, "test arg", CliType::String, true) },
      {}
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_EQ(command.getArgs().size(), 1);

  Arg arg = command.getArgs()[0];
  EXPECT_EQ(arg.getName(), CliArgName { CliLongName("file") });
  EXPECT_EQ(arg.getValue(), CliValue { std::string { "test.txt" } });

  EXPECT_FALSE(command.getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserParsesCommandWithMultipleArgs) {
  int offset(0);
  std::vector< const char* > cliArgs { "command", "--file", "test.txt", "-b", "true" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      {
        ArgDefinition("file", std::nullopt, "test arg", CliType::String, true),
        ArgDefinition("bool", "b", "test arg 2", CliType::Bool, true),
      },
      {}
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_EQ(command.getArgs().size(), 2);

  Arg arg1 = command.getArgs()[0];
  EXPECT_EQ(arg1.getName(), CliArgName { CliLongName("file") });
  EXPECT_EQ(arg1.getValue(), CliValue { std::string { "test.txt" } });

  Arg arg2 = command.getArgs()[1];
  EXPECT_EQ(arg2.getName(), CliArgName { CliShortName("b") });
  EXPECT_EQ(arg2.getValue(), CliValue { bool(true) });

  EXPECT_FALSE(command.getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserParsesWithoutNonRequiredArg) {
  int offset(0);
  std::vector< const char* > cliArgs { "command" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      { ArgDefinition("file", std::nullopt, "test arg", CliType::String, false) },
      {}
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_TRUE(command.getArgs().empty());
  EXPECT_FALSE(command.getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserFailsToParseWithoutRequiredArg) {
  int offset(0);
  std::vector< const char* > cliArgs { "command" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      { ArgDefinition("file", std::nullopt, "test arg", CliType::String, true) },
      {}
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_FALSE(maybeCommand.has_value());
  EXPECT_EQ(maybeCommand.error(), CliParsingError::MissingRequiredArgument);
}

TEST(CliParserTest, CommandParserParsesCommandWithSubcommand) {
  int offset(0);
  std::vector< const char* > cliArgs { "command", "subcommand" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      {},
      { CommandDefinition("subcommand", "test subcommand", {}, {}) }
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_TRUE(command.getArgs().empty());
  EXPECT_TRUE(command.getSubcommand().has_value());

  std::unique_ptr< Command >& subcommand = command.getSubcommand().value();

  EXPECT_STREQ(subcommand->getName(), "subcommand");
  EXPECT_TRUE(subcommand->getArgs().empty());
  EXPECT_FALSE(subcommand->getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserParsesCommandWithMultipleSubcommands) {
  int offset(0);
  std::vector< const char* > cliArgs { "command", "subcommand2" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      {},
      {
        CommandDefinition("subcommand1", "test subcommand1", {}, {}),
        CommandDefinition("subcommand2", "test subcommand2", {}, {})
      }
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_TRUE(command.getArgs().empty());
  EXPECT_TRUE(command.getSubcommand().has_value());

  std::unique_ptr< Command >& subcommand = command.getSubcommand().value();

  EXPECT_STREQ(subcommand->getName(), "subcommand2");
  EXPECT_TRUE(subcommand->getArgs().empty());
  EXPECT_FALSE(subcommand->getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserParsesWithoutRequiringSubcommand) {
  int offset(0);
  std::vector< const char* > cliArgs { "command" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      {},
      {
        CommandDefinition("subcommand1", "test subcommand1", {}, {}),
        CommandDefinition("subcommand2", "test subcommand2", {}, {})
      }
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
  EXPECT_TRUE(maybeCommand.value().has_value());

  Command& command = maybeCommand.value().value();

  EXPECT_STREQ(command.getName(), "command");
  EXPECT_TRUE(command.getArgs().empty());
  EXPECT_FALSE(command.getSubcommand().has_value());
}

TEST(CliParserTest, CommandParserIgnoresUnknownSubcommands) {
  int offset(0);
  std::vector< const char* > cliArgs { "command", "subcommand" };
  std::vector< CommandDefinition > commandDefs {
    CommandDefinition(
      "command",
      "test command",
      {},
      {}
    )
  };

  CommandParser commandParser(&offset, std::span { cliArgs }, std::span { commandDefs });
  std::expected< std::optional< Command >, CliParsingError> maybeCommand = commandParser.parse();

  EXPECT_TRUE(maybeCommand.has_value());
}

// ============================= End-to-end Tests ==============================
TEST(CliParserTest, EndToEndCliParserTest) {
  std::expected< Parser, CliDefinitionError > maybeCliParser = ParserBuilder()
    .withCommand(
      CommandDefinitionBuilder()
        .withName("command")
        ->withHelp("test help msg")
        ->withArg(
          ArgDefinitionBuilder()
            .withLongName("commandArg1")
            ->withHelp("test help msg for commandArg1")
            ->withType(CliType::U64)
            ->build()
            .value()
        )
        ->withSubcommand(
          CommandDefinitionBuilder()
            .withName("subcommand1")
            ->withHelp("test help msg for subcommand1")
            ->withArg(
              ArgDefinitionBuilder()
                .withLongName("subcommand1Arg1")
                ->withHelp("test help msg for subcommand1Arg1")
                ->withType(CliType::I64)
                ->build()
                .value()
            )
            ->build()
            .value()
        )
        ->withSubcommand(
          CommandDefinitionBuilder()
            .withName("subcommand2")
            ->withHelp("test help msg for subcommand2")
            ->withSubcommand(
              CommandDefinitionBuilder()
                .withName("subcommand2subcommand1")
                ->withHelp("test help msg for subcommand2subcommand1")
                ->build()
                .value()
            )
            ->build()
            .value()
        )
        ->build()
        .value()
      )
  ->build();
  EXPECT_TRUE(maybeCliParser.has_value());
  Parser cliParser = maybeCliParser.value();

  int argc1 = 7;
  const char* argv1[] = { "my_test", "command", "--commandArg1", "42", "subcommand1", "--subcommand1Arg1", "-42" };

  std::expected< Command, CliParsingError > maybeCommand1 = cliParser.parse(argc1, argv1);

  EXPECT_TRUE(maybeCommand1.has_value());
  Command& command1 = maybeCommand1.value();

  EXPECT_STREQ(command1.getName(), "command");
  EXPECT_EQ(command1.getArgs().size(), 1);

  Arg command1arg1 = command1.getArgs()[0];
  EXPECT_EQ(command1arg1.getName(), CliArgName { CliLongName("commandArg1") });
  EXPECT_EQ(command1arg1.getValue(), CliValue { uint64_t(42) });

  EXPECT_TRUE(command1.getSubcommand().has_value());

  std::unique_ptr< Command >& command1subcommand1 = command1.getSubcommand().value();

  EXPECT_STREQ(command1subcommand1->getName(), "subcommand1");
  EXPECT_EQ(command1subcommand1->getArgs().size(), 1);
  EXPECT_FALSE(command1subcommand1->getSubcommand().has_value());

  Arg command1subcommand1Arg1 = command1subcommand1->getArgs()[0];
  EXPECT_EQ(command1subcommand1Arg1.getName(), CliArgName { CliLongName("subcommand1Arg1") });
  EXPECT_EQ(command1subcommand1Arg1.getValue(), CliValue { int64_t(-42) });

  int argc2 = 4;
  const char* argv2[] = { "my_test", "command", "subcommand2", "subcommand2subcommand1" };

  cliParser.reset();
  std::expected< Command, CliParsingError > maybeCommand2 = cliParser.parse(argc2, argv2);

  EXPECT_TRUE(maybeCommand2.has_value());
  Command& command2 = maybeCommand2.value();

  EXPECT_STREQ(command2.getName(), "command");
  EXPECT_TRUE(command2.getArgs().empty());
  EXPECT_TRUE(command2.getSubcommand().has_value());

  std::unique_ptr< Command >& command2subcommand2 = command2.getSubcommand().value();
  EXPECT_STREQ(command2subcommand2->getName(), "subcommand2");
  EXPECT_TRUE(command2subcommand2->getArgs().empty());
  EXPECT_TRUE(command2subcommand2->getSubcommand().has_value());

  std::unique_ptr< Command >& command2subcommand2subcommand1 = command2subcommand2->getSubcommand().value();
  EXPECT_STREQ(command2subcommand2subcommand1->getName(), "subcommand2subcommand1");
  EXPECT_TRUE(command2subcommand2subcommand1->getArgs().empty());
  EXPECT_FALSE(command2subcommand2subcommand1->getSubcommand().has_value());
}
