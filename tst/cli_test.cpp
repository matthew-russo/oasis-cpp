#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "interaction/cli/parser.hpp"

using namespace oasis::interaction::cli;

TEST(CliParserTest, ArgDefinitionWithNoNameReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliParserError > maybeArgDef = builder
    .withHelp("test help msg")
    ->withType(CliType::Bool)
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliParserError::DefinitionMissingName);
}

TEST(CliParserTest, ArgDefinitionWithNoHelpReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliParserError > maybeArgDef = builder
    .withLongName("name")
    ->withType(CliType::Bool)
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliParserError::DefinitionMissingHelpMessage);
}

TEST(CliParserTest, ArgDefinitionWithNoTypeReturnsError) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliParserError > maybeArgDef = builder
    .withLongName("name")
    ->withHelp("test help msg")
    ->build();
  EXPECT_FALSE(maybeArgDef.has_value());
  EXPECT_EQ(maybeArgDef.error(), CliParserError::DefinitionMissingType);
}

TEST(CliParserTest, SuccessfulArgDefinition) {
  ArgDefinitionBuilder builder;
  std::expected< ArgDefinition, CliParserError > maybeArgDef = builder
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
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
    .withHelp("test help msg")
    ->build();
  EXPECT_FALSE(maybeCommandDef.has_value());
  EXPECT_EQ(maybeCommandDef.error(), CliParserError::DefinitionMissingName);
}

TEST(CliParserTest, CommandDefinitionWithNoHelpReturnsError) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
    .withName("name")
    ->build();
  EXPECT_FALSE(maybeCommandDef.has_value());
  EXPECT_EQ(maybeCommandDef.error(), CliParserError::DefinitionMissingHelpMessage);
}

TEST(CliParserTest, SuccessfulBasicCommandDefinition) {
  CommandDefinitionBuilder builder;
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
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
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
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
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
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
  std::expected< CommandDefinition, CliParserError > maybeCommandDef = builder
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
  std::expected< Parser, CliParserError > maybeParser = builder.build();
  EXPECT_FALSE(maybeParser.has_value());
  EXPECT_EQ(maybeParser.error(), CliParserError::AtLeastOneCommandRequired);
}

// TEST(CliParserTest, CanParseEmptyDefinition) {
//   ParserBuilder builder;
//   Parser parser = builder.build();
//   parser.parse(1, "test");
// }
