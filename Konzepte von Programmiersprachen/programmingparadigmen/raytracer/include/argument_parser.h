#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>
#include <sstream>

/**
 * @brief Enumerates the types of arguments.
 */
enum class ArgumentType {
    /**
     * @brief Argument takes a value after a prefix (e.g., --name value).
     */
    PREFIX_VALUE,
    /**
     * @brief Argument is a single flag (e.g., --verbose).
     */
    SINGLE_VALUE
};

/**
 * @brief Represents a command-line argument.
 */
class Argument {
public:
    /**
     * @brief Type alias for the argument's value.  Supports string, boolean, int, float, and double.
     */
    using Value = std::variant<std::optional<std::string>, std::optional<bool>, std::optional<int>, std::optional<float>, std::optional<double>>;

    /**
     * @brief Constructs an Argument object.
     * @param names A vector of strings representing the argument's names (e.g., {"-n", "--name"}).
     * @param is_required True if the argument is required, false otherwise.
     * @param type The type of the argument (PREFIX_VALUE or SINGLE_VALUE).
     * @param default_value The default value for the argument.
     * @param description A description of the argument.
     */
    Argument(const std::vector<std::string>& names, bool is_required, ArgumentType type, Value default_value,
             const std::string& description = "")
        : names(names), is_required(is_required), type(type), value(default_value), description(description) {}

    /**
     * @brief Parses the argument from the command-line arguments.
     * @param argc The number of command-line arguments.
     * @param argv An array of C-style strings representing the command-line arguments.
     * @return True if the argument was found and parsed, false otherwise.
     * @throws std::runtime_error If there is an error parsing the argument.
     */
    bool parse(int argc, char** argv) {
        for (int i = 0; i < argc; ++i) {
            for (const auto& name : names) {
                if (std::string(argv[i]) == name) {
                    if (type == ArgumentType::PREFIX_VALUE) {
                        if (i + 1 < argc) {
                            try {
                                if (std::holds_alternative<std::optional<std::string>>(value)) {
                                    value = std::optional<std::string>(argv[i + 1]);
                                } else if (std::holds_alternative<std::optional<int>>(value)) {
                                    value = std::optional<int>(std::stoi(argv[i + 1]));
                                } else if (std::holds_alternative<std::optional<double>>(value)) {
                                    value = std::optional<double>(std::stod(argv[i + 1]));
                                } else if (std::holds_alternative<std::optional<float>>(value)) {
                                    value = std::optional<float>(std::stof(argv[i + 1]));
                                }
                            } catch (const std::exception& e) {
                                throw std::runtime_error("Invalid value for argument: " + name + ": " + e.what());
                            }
                            parsed = true; //Flag as successfully parsed
                            return true;
                        } else {
                            throw std::runtime_error("Option requires a value: " + name);
                        }
                    } else if (type == ArgumentType::SINGLE_VALUE) {
                        if (std::holds_alternative<std::optional<bool>>(value)) {
                            value = std::optional<bool>(true);
                        } else {
                            throw std::runtime_error("Invalid boolean flag usage: " + name);
                        }
                        parsed = true; //Flag as successfully parsed
                        return true;
                    }
                }
            }
        }

        if (is_required) {
            bool value_set = false;
            if (std::holds_alternative<std::optional<std::string>>(value)) {
                value_set = std::get<std::optional<std::string>>(value).has_value();
            } else if (std::holds_alternative<std::optional<int>>(value)) {
                value_set = std::get<std::optional<int>>(value).has_value();
            } else if (std::holds_alternative<std::optional<double>>(value)) {
                value_set = std::get<std::optional<double>>(value).has_value();
            } else if (std::holds_alternative<std::optional<float>>(value)) {
                value_set = std::get<std::optional<float>>(value).has_value();
            } else if (std::holds_alternative<std::optional<bool>>(value)) {
                value_set = std::get<std::optional<bool>>(value).has_value(); // Check if boolean flag was set
            }

            if (!value_set) {
                throw std::runtime_error("Missing required option: " + names[0]);
            }
        }
        return false;
    }

    /**
     * @brief Gets the value of the argument.
     * @tparam T The type of the value to retrieve (string, bool, int, float, or double).
     * @return An std::optional containing the value if it has been set and is of the correct type, std::nullopt otherwise.
     */
    template <typename T>
    std::optional<T> get() const {
        if constexpr (std::is_same_v<T, std::string>) {
            return std::get<std::optional<std::string>>(value);
        } else if constexpr (std::is_same_v<T, int>) {
            return std::get<std::optional<int>>(value);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::get<std::optional<double>>(value);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::get<std::optional<float>>(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return std::get<std::optional<bool>>(value);
        }
        return std::nullopt;
    }

    /**
     * @brief Parses all arguments from the command line.
     * @param arguments A vector of Argument objects.
     * @param argc The number of command-line arguments.
     * @param argv An array of C-style strings representing the command-line arguments.
     * @return True if all arguments were parsed successfully. Exception is thrown otherwise.
     */
    static bool parseArguments(std::vector<Argument>& arguments, int argc, char** argv) {
        try {
            for (auto& arg : arguments) {
                arg.parse(argc, argv);
            }
        } catch (const std::runtime_error& e) {
            throw e;
        }
        return true;
    }

    /**
     * @brief Prints the usage information for the arguments.
     * @param arguments A vector of Argument objects.
     * @param out The output stream to print to (default is std::cout).
     */
    static void usage(const std::vector<Argument>& arguments, std::ostream& out = std::cout) {
        out << "Options:\n";
        for (const auto& arg : arguments) {
            std::string default_value_str = arg.defaultValueAsString();
            out << formatArgument(arg.names[0], default_value_str, arg.description, !arg.is_required);
        }
    }

    /**
     * @brief Checks for the help flag and prints usage if found.
     * @param argc The number of command-line arguments.
     * @param argv An array of C-style strings representing the command-line arguments.
     * @param arguments A vector of Argument objects.
     * @param out The output stream to print to (default is std::cout).
     * @return True if the help flag was found, false otherwise.
     */
    static bool checkHelpFlag(int argc, char** argv, const std::vector<Argument>& arguments, std::ostream& out = std::cout) {
        for (int i = 0; i < argc; ++i) {
            std::string arg(argv[i]);
            if (arg == "-h" || arg == "--help") {
                usage(arguments, out);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Warns about unrecognised arguments if any
     * @param argc The number of command-line arguments.
     * @param argv An array of C-style strings representing the command-line arguments.
     * @param arguments A vector of Argument objects.
     * @param out The output stream to print to (default is std::cerr).
     * @return void
     */
    static void warnUnrecognized(int argc, char** argv, const std::vector<Argument>& arguments, std::ostream& out = std::cerr) {
         std::vector<std::string> recognizedArgs;

         for (const auto& arg : arguments) {
            recognizedArgs.insert(recognizedArgs.end(), arg.names.begin(), arg.names.end());
        }

        for (int i = 1; i < argc; ++i) { // Start from 1 to skip the executable name
            std::string arg(argv[i]);
            bool isRecognized = false;
            for (const auto& recognized : recognizedArgs) {
                if (arg == recognized) {
                    isRecognized = true;
                    break;
                }
            }
            if (!isRecognized && arg[0] == '-') { // Only warn for arguments that start with '-'
                out << "Warning: Unrecognized argument: " << arg << std::endl;
            }
        }
    }

private:
    std::vector<std::string> names;
    bool is_required;
    ArgumentType type;
    Value value;
    std::string description;
    bool parsed = false; ///< Flag to indicate whether the argument has been successfully parsed.

    /**
     * @brief Returns the default value of the argument as a string.
     * @return The default value as a string.
     */
    std::string defaultValueAsString() const {
        if (std::holds_alternative<std::optional<std::string>>(value)) {
            auto val = std::get<std::optional<std::string>>(value);
            return val.has_value() ? *val : ""; // Use * to dereference optional
        } else if (std::holds_alternative<std::optional<int>>(value)) {
            auto val = std::get<std::optional<int>>(value);
            return val.has_value() ? std::to_string(*val) : "";
        } else if (std::holds_alternative<std::optional<float>>(value)) {
            auto val = std::get<std::optional<float>>(value);
            return val.has_value() ? std::to_string(*val) : "";
        } else if (std::holds_alternative<std::optional<double>>(value)) {
            auto val = std::get<std::optional<double>>(value);
            return val.has_value() ? std::to_string(*val) : "";
        } else if (std::holds_alternative<std::optional<bool>>(value)) {
            auto val = std::get<std::optional<bool>>(value);
            return val.has_value() ? (*val ? "true" : "false") : "";
        }
        return "";
    }

    /**
     * @brief Formats an argument for the usage message.
     * @param name The name of the argument.
     * @param value The default value of the argument.
     * @param desc The description of the argument.
     * @param optional True if the argument is optional, false otherwise.
     * @return The formatted argument string.
     */
    static std::string formatArgument(const std::string& name, const std::string& value, const std::string& desc, bool optional) {
        std::ostringstream oss;
        oss << name;
        if (!value.empty()) {
            oss << " " << value;
        }
        int padding = 30 - name.size() - value.size();
        if (padding > 0) { // Only add padding if it's positive.
            oss << std::string(padding, ' ');
        }
        oss << "= " << desc << (optional ? " (optional)" : " (required)") << "\n";
        return oss.str();
    }
};

#endif