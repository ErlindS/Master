#include <gtest/gtest.h>
#include "argument_parser.h"

// Test case for parsing an integer argument
TEST(ArgumentTest, ParseIntegerArgument) {
    const char* argv[] = {"program", "-no_samples", "4"};
    int argc = 3;

    Argument arg({"-no_samples"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(0));
    EXPECT_NO_THROW(arg.parse(argc, const_cast<char**>(argv)));
    EXPECT_EQ(arg.get<int>().value_or(0), 4);
}

// Test case for parsing a string argument
TEST(ArgumentTest, ParseStringArgument) {
    const char* argv[] = {"program", "-ppm_file_name", "output.ppm"};
    int argc = 3;

    Argument arg({"-ppm_file_name"}, true, ArgumentType::PREFIX_VALUE, std::optional<std::string>("default.ppm"));
    EXPECT_NO_THROW(arg.parse(argc, const_cast<char**>(argv)));
    EXPECT_EQ(arg.get<std::string>().value_or("default.ppm"), "output.ppm");
}

// Test case for parsing a boolean flag argument
TEST(ArgumentTest, ParseBooleanFlag) {
    const char* argv[] = {"program", "-hit_buffer_is_on"};
    int argc = 2;

    Argument arg({"-hit_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false));
    EXPECT_NO_THROW(arg.parse(argc, const_cast<char**>(argv)));
    EXPECT_EQ(arg.get<bool>().value_or(false), true);
}

// Test case for missing boolean flag (should default to false)
TEST(ArgumentTest, BooleanFlagNotProvided) {
    const char* argv[] = {"program"};
    int argc = 1;

    Argument arg({"-hit_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false));
    EXPECT_NO_THROW(arg.parse(argc, const_cast<char**>(argv)));
    EXPECT_EQ(arg.get<bool>().value_or(false), false);
}

// Test case for invalid integer argument
TEST(ArgumentTest, InvalidIntegerValue) {
    const char* argv[] = {"program", "-no_samples", "invalid"};
    int argc = 3;

    Argument arg({"-no_samples"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(0));
    EXPECT_THROW(arg.parse(argc, const_cast<char**>(argv)), std::runtime_error);
}

// Test case for missing required argument
TEST(ArgumentTest, MissingRequiredArgument) {
    const char* argv[] = {"program"};
    int argc = 1;

    Argument arg({"-width"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(800));
	EXPECT_EQ(arg.get<int>().value_or(800), 800);
}

// Full raytracer command-line test
TEST(ArgumentTest, RaytracerCommandLineTest) {
    const char* argv[] = {
        "raytracer", "-no_samples", "1", "-width", "800", "-height", "600",
        "-ppm_file_name", "output.ppm", "-hit_buffer_is_on", "-shadow_buffer_is_on"
    };
    int argc = 11;

    std::vector<Argument> arguments = {
        Argument({"-no_samples"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(0)),
        Argument({"-width"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(800)),
        Argument({"-height"}, true, ArgumentType::PREFIX_VALUE, std::optional<int>(600)),
        Argument({"-ppm_file_name"}, true, ArgumentType::PREFIX_VALUE, std::optional<std::string>("output.ppm")),
        Argument({"-hit_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false)),
        Argument({"-shadow_buffer_is_on"}, false, ArgumentType::SINGLE_VALUE, std::optional<bool>(false))
    };

    EXPECT_NO_THROW(Argument::parseArguments(arguments, argc, const_cast<char**>(argv)));

    EXPECT_EQ(arguments[0].get<int>().value_or(0), 1);
    EXPECT_EQ(arguments[1].get<int>().value_or(800), 800);
    EXPECT_EQ(arguments[2].get<int>().value_or(600), 600);
    EXPECT_EQ(arguments[3].get<std::string>().value_or("default.ppm"), "output.ppm");
    EXPECT_EQ(arguments[4].get<bool>().value_or(false), true);
    EXPECT_EQ(arguments[5].get<bool>().value_or(false), true);
}
