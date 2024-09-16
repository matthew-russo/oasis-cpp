#ifndef OASIS_INTERACTION_CLI_PARSER_H
#define OASIS_INTERACTION_CLI_PARSER_H

#include <expected>
#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace oasis {
namespace interaction {
namespace cli {

enum class CliParserError {
  DefinitionMissingName,
  DefinitionMissingHelpMessage,
  DefinitionMissingType,
  AtLeastOneCommandRequired,
};

enum class CliType {
  U64,
  I64,
  Bool,
  String,
};

using CliValue = std::variant<
  uint64_t,
  int64_t,
  bool,
  std::string
>;

class Arg {
  char const * longName;
  std::optional< char const * > shortName;
  CliValue value;

public:
  char const * getLongName() {
    return this->longName;
  }

  std::optional< char const * > getShortName() {
    return this->shortName;
  }

  CliValue getValue() {
    return this->value;
  }
};

class ArgDefinition {
  char const * longName;
  std::optional< char const * > shortName;
  char const * help;
  CliType type;
  bool required;

public:
  ArgDefinition(
    char const * longName,
    std::optional< char const * > shortName,
    char const * help,
    CliType type,
    bool required
  ) :
    longName(longName),
    shortName(shortName),
    help(help),
    type(type),
    required(required) {}

  char const * getLongName() {
    return this->longName;
  }

  std::optional< char const * > getShortName() {
    return this->shortName;
  }

  char const * getHelp() {
    return this->help;
  }

  CliType getType() {
    return this->type;
  }

  bool isRequired() {
    return this->required;
  }
};

class ArgDefinitionBuilder {
  std::optional< char const * > longName;
  std::optional< char const * > shortName;
  std::optional< char const * > help;
  std::optional< CliType > type;
  bool required;

public:
  ArgDefinitionBuilder* withLongName(char const * longName) {
    this->longName = longName;
    return this;
  }

  ArgDefinitionBuilder* withShortName(char const * shortName) {
    this->shortName = shortName;
    return this;
  }

  ArgDefinitionBuilder* withHelp(char const * help) {
    this->help = help;
    return this;
  }

  ArgDefinitionBuilder* withType(CliType type) {
    this->type = type;
    return this;
  }

  ArgDefinitionBuilder* isRequired(bool required) {
    this->required = required;
    return this;
  }

  std::expected< ArgDefinition, CliParserError > build() {
    if (!this->longName.has_value()) {
      return std::unexpected(CliParserError::DefinitionMissingName);
    }

    if (!this->help.has_value()) {
      return std::unexpected(CliParserError::DefinitionMissingHelpMessage);
    }

    if (!this->type.has_value()) {
      return std::unexpected(CliParserError::DefinitionMissingType);
    }

    return ArgDefinition(
      this->longName.value(),
      this->shortName,
      this->help.value(),
      this->type.value(),
      this->required
    );
  }
};

class Command {
  char const * name;
  std::vector< Arg > args;
  std::vector< Command > subcommands;

public:
  char const * getName() {
    return this->name;
  }

  std::span< Arg > getArgs() {
    return std::span { this->args };
  }

  std::span< Command > getSubcommands() {
    return std::span { this->subcommands };
  }
};

class CommandDefinition {
  char const * name;
  char const * help;
  std::vector< ArgDefinition > possibleArgs;
  std::vector< CommandDefinition > possibleSubcommands;

public:
  CommandDefinition(
    char const * name,
    char const * help,
    std::vector< ArgDefinition > possibleArgs,
    std::vector< CommandDefinition > possibleSubcommands
  ) :
    name(name),
    help(help),
    possibleArgs(possibleArgs),
    possibleSubcommands(possibleSubcommands) {}

  char const * getName() {
    return this->name;
  }

  char const * getHelp() {
    return this->help;
  }

  std::span< ArgDefinition > getPossibleArgs() {
    return std::span { this->possibleArgs };
  }

  std::span< CommandDefinition > getPossibleSubcommands() {
    return std::span { this->possibleSubcommands };
  }
};

class CommandDefinitionBuilder {
  std::optional< char const * > name;
  std::optional< char const * > help;
  std::vector< ArgDefinition > possibleArguments;
  std::vector< CommandDefinition > possibleSubcommands;

public:
  CommandDefinitionBuilder* withName(char const * name) {
    assert(name != nullptr);
    this->name = name;
    return this;
  }

  CommandDefinitionBuilder* withHelp(char const * help) {
    assert(name != nullptr);
    this->help = help;
    return this;
  }

  CommandDefinitionBuilder* withArg(ArgDefinition arg) {
    this->possibleArguments.emplace_back(arg);
    return this;
  }

  CommandDefinitionBuilder* withSubcommand(CommandDefinition subcommand) {
    this->possibleSubcommands.emplace_back(subcommand);
    return this;
  }

  std::expected< CommandDefinition, CliParserError > build() {
    if (!this->name.has_value()) {
      return std::unexpected(CliParserError::DefinitionMissingName);
    }

    if (!this->help.has_value()) {
      return std::unexpected(CliParserError::DefinitionMissingHelpMessage);
    }

    return CommandDefinition(this->name.value(), this->help.value(), this->possibleArguments, this->possibleSubcommands);
  }
};

class Parser {
  std::vector< CommandDefinition > possibleCommands;

public:
  Parser(std::vector< CommandDefinition > possibleCommands)
    : possibleCommands(possibleCommands) {}

  ///
  /// argc is the number of command arguments, unmodified from what main gives.
  /// it is always at least 1 because by convention, program name is always provided
  /// as the first argument
  ///
  /// argv are the space-separated arguments, the first entry is always the program
  /// name
  ///
  std::expected< Command, CliParserError > parse(int argc, char *argv[]) {
    throw std::runtime_error("todo: finish impl");
  }
};

class ParserBuilder {
  std::vector< CommandDefinition > possibleCommands;

public:
  ParserBuilder* withCommand(CommandDefinition commandDef) {
    this->possibleCommands.emplace_back(commandDef);
    return this;
  }

  std::expected< Parser, CliParserError > build() {
    if (this->possibleCommands.empty()) {
      return std::unexpected(CliParserError::AtLeastOneCommandRequired);
    }

    return Parser(std::move(this->possibleCommands));
  }
};

}; // namespace cli
}; // namespace interaction
}; // namespace oasis

#endif // OASIS_INTERACTION_CLI_PARSER_H
