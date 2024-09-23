#ifndef OASIS_INTERACTION_CLI_PARSER_H
#define OASIS_INTERACTION_CLI_PARSER_H

#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string.h>
#include <variant>
#include <vector>

#include "../../utils.hpp"

namespace oasis {
namespace interaction {
namespace cli {

enum class CliDefinitionError {
  DefinitionMissingName,
  DefinitionMissingHelpMessage,
  DefinitionMissingType,
  AtLeastOneCommandRequired,
};

enum class CliParsingError {
  UnknownCommand,
  UnknownArgument,
  MissingRequiredArgument,
  MissingCommand,
  InvalidBooleanValue,
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

class CliShortName {
  const char* name;

public:
  CliShortName(const char* name) : name(name) {}

  const char* getName() const {
    return this->name;
  }

  bool operator==(const CliShortName& other) const {
    return 0 == strcmp(this->name, other.name);
  }
};

class CliLongName {
  const char* name;

public:
  CliLongName(const char* name) : name(name) {}

  const char* getName() const {
    return this->name;
  }

  bool operator==(const CliLongName& other) const {
    return 0 == strcmp(this->name, other.name);
  }
};

using CliArgName = std::variant<
  CliLongName,
  CliShortName
>;

class Arg {
  CliArgName name;
  CliValue value;

public:
  Arg(CliArgName name, CliValue value) : name(name), value(value) {}
  
  const CliArgName& getName() const {
    return this->name;
  }

  CliValue getValue() const {
    return this->value;
  }
};

class ArgDefinition {
  char const* longName;
  std::optional< char const* > shortName;
  char const* help;
  CliType type;
  bool required;

public:
  ArgDefinition(
    char const* longName,
    std::optional< char const* > shortName,
    char const* help,
    CliType type,
    bool required
  ) :
    longName(longName),
    shortName(shortName),
    help(help),
    type(type),
    required(required) {}

  char const* getLongName() const {
    return this->longName;
  }

  std::optional< char const* > getShortName() const {
    return this->shortName;
  }

  char const* getHelp() const {
    return this->help;
  }

  CliType getType() const {
    return this->type;
  }

  bool isRequired() const {
    return this->required;
  }

  bool matchesArgName(const CliArgName& argName) const {
    return std::visit(Overload {
      [this](const CliLongName& longName) {
        return 0 == strcmp(longName.getName(), this->longName);
      },
      [this](const CliShortName& shortName) {
        if (this->shortName.has_value()) {
          return 0 == strcmp(shortName.getName(), this->shortName.value());
        } else {
          return false;
        }
      }
    }, argName);
  }
};

class ArgDefinitionBuilder {
  std::optional< char const* > longName;
  std::optional< char const* > shortName;
  std::optional< char const* > help;
  std::optional< CliType > type;
  bool required;

public:
  ArgDefinitionBuilder* withLongName(char const* longName) {
    this->longName = longName;
    return this;
  }

  ArgDefinitionBuilder* withShortName(char const* shortName) {
    this->shortName = shortName;
    return this;
  }

  ArgDefinitionBuilder* withHelp(char const* help) {
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

  std::expected< ArgDefinition, CliDefinitionError > build() {
    if (!this->longName.has_value()) {
      return std::unexpected(CliDefinitionError::DefinitionMissingName);
    }

    if (!this->help.has_value()) {
      return std::unexpected(CliDefinitionError::DefinitionMissingHelpMessage);
    }

    if (!this->type.has_value()) {
      return std::unexpected(CliDefinitionError::DefinitionMissingType);
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
  char const* name;
  std::vector< Arg > args;
  std::optional< std::unique_ptr< Command > > subcommand;

public:
  Command(char const* name, std::vector< Arg >&& args, std::optional< std::unique_ptr< Command >>&& subcommand) :
    name(name),
    args(std::move(args)),
    subcommand(std::move(subcommand)) {}

  char const* getName() {
    return this->name;
  }

  std::span< Arg > getArgs() {
    return std::span { this->args };
  }

  std::optional< std::unique_ptr< Command >>& getSubcommand() {
    return this->subcommand;
  }
};

class CommandDefinition {
  char const* name;
  char const* help;
  std::vector< ArgDefinition > possibleArgs;
  std::vector< CommandDefinition > possibleSubcommands;

public:
  CommandDefinition(
    char const* name,
    char const* help,
    std::vector< ArgDefinition > possibleArgs,
    std::vector< CommandDefinition > possibleSubcommands
  ) :
    name(name),
    help(help),
    possibleArgs(possibleArgs),
    possibleSubcommands(possibleSubcommands) {}

  char const* getName() {
    return this->name;
  }

  char const* getHelp() {
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
  std::optional< char const* > name;
  std::optional< char const* > help;
  std::vector< ArgDefinition > possibleArguments;
  std::vector< CommandDefinition > possibleSubcommands;

public:
  CommandDefinitionBuilder* withName(char const* name) {
    assert(name != nullptr);
    this->name = name;
    return this;
  }

  CommandDefinitionBuilder* withHelp(char const* help) {
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

  std::expected< CommandDefinition, CliDefinitionError > build() {
    if (!this->name.has_value()) {
      return std::unexpected(CliDefinitionError::DefinitionMissingName);
    }

    if (!this->help.has_value()) {
      return std::unexpected(CliDefinitionError::DefinitionMissingHelpMessage);
    }

    return CommandDefinition(this->name.value(), this->help.value(), this->possibleArguments, this->possibleSubcommands);
  }
};

class ArgParser {
  int* offset;
  std::span< const char* > cliArgs;
  std::span< ArgDefinition > validArgs;

public:
  ArgParser(
    int* offset,
    std::span< const char* > cliArgs,
    std::span< ArgDefinition > validArgs
  ) : offset(offset), cliArgs(cliArgs), validArgs(validArgs) {}

  std::expected< std::optional< Arg >, CliParsingError> parse() {
    if (*offset >= cliArgs.size()) {
      return std::nullopt;
    }

    const char* name = this->cliArgs[*offset];
    size_t len = strlen(name);
    assert(len >= 2);

    // args always start with "-",
    //   - short args are just a single "-", e.g. '-f'
    //   - large args are two "-", e.g. '--file'
    if (name[0] != '-') {
      // if the next word does not start with a '-', we're not parsing an arg
      return std::nullopt;
    }

    *offset += 1;

    // check the second char to determine whether we're parsing a long or short arg
    // if the second char is another hyphen, we're parsing a long arg
    std::optional< CliArgName > argName;
    if (name[1] == '-') {
      name += 2;
      argName = CliLongName(name);
    } else {
    // otherwise we're parsing a short arg
      name += 1;
      argName = CliShortName(name);
    }

    assert(*offset < this->cliArgs.size());
    const char* valueStr = this->cliArgs[*offset];
    *offset += 1;
    CliValue argValue;

    auto argMatches = [argName](const ArgDefinition& argDef) {
      return argDef.matchesArgName(argName.value());
    };
    if (auto it = std::find_if(this->validArgs.begin(), this->validArgs.end(), argMatches); it != this->validArgs.end()) {
      ArgDefinition argDef = *it;
      char* end = nullptr;
      switch (argDef.getType()) {
        case CliType::U64:
          argValue = std::strtoull(valueStr, &end, 10 /* base 10 */);
          // TODO handle errors
          break;
        case CliType::I64:
          argValue = std::strtoll(valueStr, &end, 10 /* base 10 */);
          // TODO handle errors
          break;
        case CliType::Bool:
          if (0 == strcmp(valueStr, "true")) {
            argValue = true;
          } else if (0 == strcmp(valueStr, "false")) {
            argValue = false;
          } else {
            return std::unexpected(CliParsingError::InvalidBooleanValue);
          }
          break;
        case CliType::String:
          argValue = std::string(valueStr);
          break;
      }
    } else {
      return std::unexpected(CliParsingError::UnknownArgument);
    }

    return Arg(argName.value(), argValue);
  }
};

class CommandParser {
  int* offset;
  std::span< const char* > cliArgs;
  std::span< CommandDefinition > validCommands;

public:
  CommandParser(
    int* offset,
    std::span< const char* > cliArgs,
    std::span< CommandDefinition > validCommands
  ) : offset(offset), cliArgs(cliArgs), validCommands(validCommands) {}

  std::expected< std::optional< Command >, CliParsingError> parse() {
    if (*offset >= cliArgs.size()) {
      return std::nullopt;
    }

    if (this->validCommands.empty()) {
      return std::nullopt;
    }

    std::vector< Arg > args;
    char const* name = this->cliArgs[*offset];
    *offset += 1;

    auto nameMatches = [name](CommandDefinition& commandDef) {
      return 0 == strcmp(commandDef.getName(), name);
    };
    std::optional< CommandDefinition > targetCommandDefinition = std::nullopt;
    if (auto it = std::find_if(this->validCommands.begin(), this->validCommands.end(), nameMatches); it != this->validCommands.end()) {
      targetCommandDefinition = *it;
    } else {
      return std::unexpected(CliParsingError::UnknownCommand);
    }

    // 1. chomp all arguments
    while (true) {
      ArgParser argParser(
        this->offset,
        this->cliArgs,
        targetCommandDefinition.value().getPossibleArgs()
      );
      std::expected< std::optional< Arg >, CliParsingError > maybeArg = argParser.parse();

      if (!maybeArg.has_value()) {
        return std::unexpected(maybeArg.error());
      }

      if (maybeArg.value().has_value()) {
        args.emplace_back(maybeArg.value().value());
      } else {
        break;
      }
    }

    // 2. make sure all required arguments have been populated. if not, return
    // an error
    for (const ArgDefinition& argDef : targetCommandDefinition.value().getPossibleArgs()) {
      // if the argument is required, ensure its in our list of arguments
      if (argDef.isRequired()) {
        auto argMatches = [argDef](const Arg& arg) {
          return argDef.matchesArgName(arg.getName());
        };
        if (auto it = std::find_if(args.begin(), args.end(), argMatches); it != args.end()) {
          // the required arg is present, move on
          continue;
        }

        return std::unexpected(CliParsingError::MissingRequiredArgument);
      }
    }

    // 3. chomp subcommand if available
    CommandParser subcommandParser(
      this->offset,
      this->cliArgs,
      targetCommandDefinition.value().getPossibleSubcommands()
    );
    std::expected< std::optional< Command >, CliParsingError > maybeSubcommand = subcommandParser.parse();
    if (!maybeSubcommand.has_value()) {
      return std::unexpected(maybeSubcommand.error());
    }

    std::optional< std::unique_ptr< Command >> subcommand = std::nullopt;
    if (maybeSubcommand.value().has_value()) {
      subcommand = std::make_unique< Command >(std::move(maybeSubcommand.value().value()));
    }

    return Command(name, std::move(args), std::move(subcommand));
  }
};

class Parser {
  std::vector< CommandDefinition > possibleCommands;
  int offset = 0;

public:
  Parser(std::vector< CommandDefinition > possibleCommands)
    : possibleCommands(possibleCommands) {}

  void reset() {
    this->offset = 0;
  }

  ///
  /// argc is the number of command arguments, unmodified from what main gives.
  /// it is always at least 1 because by convention, program name is always provided
  /// as the first argument
  ///
  /// argv are the space-separated arguments, the first entry is always the program
  /// name. its never used but expected so that callers can pass argv from main
  /// without any modifications
  ///
  std::expected< Command, CliParsingError > parse(int argc, const char* argv[]) {
    assert(!this->possibleCommands.empty());
    // first arg is always the program name
    assert(argc > 1);

    std::vector< const char* > args(argv + 1, argv + argc);

    CommandParser parser(&this->offset, std::span { args }, std::span { this->possibleCommands });

    std::expected< std::optional< Command >, CliParsingError> maybeCommand = parser.parse();
    if (!maybeCommand.has_value()) {
      return std::unexpected(maybeCommand.error());
    }
    if (!maybeCommand.value().has_value()) {
      return std::unexpected(CliParsingError::MissingCommand);
    }
    return std::move(maybeCommand.value().value());
  }
};

class ParserBuilder {
  std::vector< CommandDefinition > possibleCommands;

public:
  ParserBuilder* withCommand(CommandDefinition commandDef) {
    this->possibleCommands.emplace_back(commandDef);
    return this;
  }

  std::expected< Parser, CliDefinitionError > build() {
    if (this->possibleCommands.empty()) {
      return std::unexpected(CliDefinitionError::AtLeastOneCommandRequired);
    }

    return Parser(std::move(this->possibleCommands));
  }
};

}; // namespace cli
}; // namespace interaction
}; // namespace oasis

#endif // OASIS_INTERACTION_CLI_PARSER_H
