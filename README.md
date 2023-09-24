# flag.h

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](<https://en.wikipedia.org/wiki/C_(programming_language)>) [![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

The Flag Library is a command-line flag parsing library written in C. It provides a simple and flexible way to parse command-line arguments and perform validation on flag values. The library supports a variety of flag types, including bool, int, size_t, int8_t, int16_t, int32_t, int64_t, unsigned int, uint8_t, uint16_t, uint32_t, uint64_t, uintptr_t, float, double, and char \*. It also allows for the addition of custom validators to validate flag values.

## Installation

To use the Flag Library in your C project, simply include the `flag.h` header file in your source code and link against the library(flag.c).

## Usage

The Flag Library provides a simple API for creating and managing flags and subcommands, parsing command-line arguments, and performing validation on flag values.

### Creating a Flag Context

To create a flag context, use the `CreateFlagContext` function:

```c
flag_ctx* ctx = CreateFlagContext();
```

This function returns a pointer to a new flag context. You can add global flags and subcommands to this context using the `AddFlag` and `AddSubCmd` macros.

### Adding Global Flags

To add a global flag to the flag context, use the `flag_add` macro:

```c
AddFlag(ctx, .name = "int", .value = &integer_flag, .type = FLAG_INT, "An integer flag");
```

This crazy macro adds a new flag to the flag context with the given name, value, type, and description. The `value` parameter should be a pointer to the variable that will hold the flag value. The `type` parameter should be one of the supported flag types. The `required` parameter should be `true` if the flag is required, and `false` otherwise.

The default value for the `required` parameter is `false`.

The default value for the `type` parameter is `FLAG_INT`. So, the above macro can be simplified to:

```c
AddFlag(ctx, .name = "int", .value = &integer_flag, "An integer flag");
```

### Adding Subcommands

To add a subcommand to the flag context, use the `AddSubCmd` macro:

```c
// Second subcommand
  subcommand *cmd2 = AddSubCmd(ctx, .name = "greet", .desc = "Greets the user", .handler = handle_greet, .capacity = 1);


```

This macro adds a new subcommand to the flag context with the given name, description, and handler function. The `handler` parameter should be a function pointer to the function that will handle the subcommand. The `capacity` parameter should be the maximum number of flags that the subcommand can have. Default value is 0.

### Adding Flags to Subcommands

To add a flag to a subcommand, use the `AddSubCmdFlag` macro:

```c
char* name = "Guest";
  AddSubCmdFlag(cmd2, .name = "name", .value = &name, .type = FLAG_STRING,
                .desc = "The name of the user to greet", );
```

This macro adds a new flag to the subcommand with the given name, value, type, description, and required status. The `value` parameter should be a pointer to the variable that will hold the flag value. The `type` parameter should be one of the supported flag types. The `required` parameter should be `true` if the flag is required, and `false` otherwise.

### Adding flag validation beyound required

```c
bool validate_int(const void* value) {
  int* int_value = (int*)value;
  return (*int_value >= 0 && *int_value <= 10);
}

// Add flags to subcommand
flag* flag_count = AddSubCmdFlag(cmd1, .name = "count", .value = &count, .type = FLAG_INT,
                        .desc = "The number of times to print hello", );

SetValidator(flag_count, validate_int, "count must be between 0 and 10");
```

### Parsing Command-Line Arguments

To parse command-line arguments, use the `ParseFlags` function:

```c
subcommand* subcmd = ParseFlags(ctx, argc, argv);
```

This function parses the command-line arguments and returns a pointer to the subcommand that was selected. If no subcommand was selected, this function returns `NULL`. You can use the `FlagValue` and `FlagValueCtx` functions to retrieve the values of flags from the selected subcommand or the global flag context respectively.

### Printing Help Messages

To print help messages for the global flags or subcommands, use the `PrintHelp` function:

```c
PrintHelp(ctx, argv);
```

This may not be neccessary as the library prints help messages automatically when the user enters invalid command-line arguments or when the user enters the `--help` flags.

### Cleaning Up

To free the memory used by the flag context, use the `DestroyFlagContext` function:

```c
DestroyFlagContext(ctx);
```

This function frees the memory used by the global flags and subcommands in the flag context.

### OVERRIDING DEFAULTS

To override and control the amount of memory allocated for subcommands and global flags, compile with the following flags:

```bash
-D MAX_SUBCOMMANDS=10 -D MAX_GLOBAL_FLAGS=10
```

To override and control the amount of memory allocated for flag names and descriptions(same for subcommands), compile with the following flags:

```bash
-D MAX_NAME=20 -D MAX_DESCRIPTION=50
```

Default values are:

- MAX_NAME = 64
- MAX_DESCRIPTION = 256
- MAX_SUBCOMMANDS = 10
- MAX_GLOBAL_FLAGS = 24

### Example

See [main.c](main.c) for a simple example of how to use the Flag Library.

## License

The Flag Library is released under the MIT License. See the `LICENSE` file for more information.
