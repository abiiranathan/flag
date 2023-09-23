# flag.h

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](<https://en.wikipedia.org/wiki/C_(programming_language)>) [![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

The Flag Library is a command-line flag parsing library written in C. It provides a simple and flexible way to parse command-line arguments and perform validation on flag values. The library supports a variety of flag types, including bool, int, size_t, int8_t, int16_t, int32_t, int64_t, unsigned int, uint8_t, uint16_t, uint32_t, uint64_t, uintptr_t, float, double, and char \*. It also allows for the addition of custom validators to validate flag values.

## Installation

To use the Flag Library in your C project, simply include the `flag.h` header file in your source code and link against the library.

## Usage

The Flag Library provides a simple API for creating and managing flags and subcommands, parsing command-line arguments, and performing validation on flag values.

### Creating a Flag Context

To create a flag context, use the `flag_context_init` function:

```c
flag_ctx* ctx = flag_context_init();
```

This function returns a pointer to a new flag context. You can add global flags and subcommands to this context using the `flag_add` and `flag_add_subcommand` functions.

### Adding Global Flags

To add a global flag to the flag context, use the `flag_add` function:

```c
flag_add(ctx, "name", &value, FLAG_TYPE, "description", required);
```

This function adds a new flag to the flag context with the given name, value, type, description, and required status. The `value` parameter should be a pointer to the variable that will hold the flag value. The `type` parameter should be one of the supported flag types. The `required` parameter should be `true` if the flag is required, and `false` otherwise.

### Adding Subcommands

To add a subcommand to the flag context, use the `flag_add_subcommand` function:

```c
subcommand* subcmd = flag_add_subcommand(ctx, "name", "description", handler, flag_capacity);
```

This function adds a new subcommand to the flag context with the given name, description, and handler function. The `handler` parameter should be a function pointer to the function that will handle the subcommand. The `flag_capacity` parameter should be the maximum number of flags that the subcommand can have.

### Adding Flags to Subcommands

To add a flag to a subcommand, use the `subcommand_add_flag` function:

```c
subcommand_add_flag(subcmd, "name", &value, FLAG_TYPE, "description", required, validator);
```

This function adds a new flag to the subcommand with the given name, value, type, description, and required status. The `value` parameter should be a pointer to the variable that will hold the flag value. The `type` parameter should be one of the supported flag types. The `required` parameter should be `true` if the flag is required, and `false` otherwise. The `validator` parameter is an optional flag validator that can be used to validate the flag value.

### Parsing Command-Line Arguments

To parse command-line arguments, use the `parse_flags` function:

```c
subcommand* subcmd = parse_flags(ctx, argc, argv);
```

This function parses the command-line arguments and returns a pointer to the subcommand that was selected. If no subcommand was selected, this function returns `NULL`. You can use the `flag_value` and `flag_value_ctx` functions to retrieve the values of flags from the selected subcommand or the global flag context.

### Performing Validation

To perform validation on flag values, you can add a flag validator to a flag or subcommand using the `flag_add_with_validator` function:

```c
flag_validator validator = {validate_function, "error message"};
flag_add_with_validator(ctx, "name", &value, FLAG_TYPE, "description", required, &validator);
```

This function adds a new flag to the flag context with the given name, value, type, description, and required status, and a flag validator that will be used to validate the flag value.

### Printing Help Messages

To print help messages for the global flags or subcommands, use the `print_help` function:

```c
print_help(ctx, argv);
```

This function prints a help message for the global flags or the selected subcommand, depending on the command-line arguments.

### Cleaning Up

To free the memory used by the flag context, use the `flag_destroy_context` function:

```c
flag_destroy_context(ctx);
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
