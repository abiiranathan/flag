# flag

## flag implements a C library for command line flag parsing.

The flag package accepts command line arguments in the form of -flag value or --arg value with custom validation and overflow/underflow checks.

The flag package supports the following types:

- bool
- int
- float32
- float64
- int8_t
- int16_t
- int32_t
- int64_t
- uint8_t
- uint16_t
- uint32_t
- uint64_t
- uintptr_t
- size_t
- unsigned int
- char \*

## Supported notation:

_flag_ supports a shorthand notation where a
flag may be specified with just a single dash (-) instead of the full --flag.

> Warning: The package does not support specifying a flag multiple times. In this case, the last value will overwrite the previous ones.

Subcommands are implemented by
providing a subcommand struct to the **_parse_flags_** function.

The subcommand struct contains a name, description, callback and flags.

No supported for nested subcommands.

The callback is then called
with the subcommand's flags and the global flag context.

This means that the subcommand may be called before all flags are parsed.
That means global flags must come before subcommands.

```c

#include "flag.h"

bool validate_int(const void* value) {
  int* int_value = (int*)value;
  return (*int_value >= 0 && *int_value <= 10);
}

void handle_subcommand(FlagArgs args) {
  int count = *(int*)flag_value(args.flags, args.num_flags, "count");
  bool verbose = *(bool*)flag_value(args.flags, args.num_flags, "verbose");

  printf("count=%d verbose=%d\n", count, verbose);

  // ctx will provide you with access to global flags
  double float64 = *(double*)flag_value_ctx(args.ctx, "float64");
  printf("float64 value in callback: %lf\n", float64);
}

void handle_greet(FlagArgs args) {
  const char* name =
    *(const char**)flag_value(args.flags, args.num_flags, "name");
  printf("Hello, %s!\n", name);
}

int main(int argc, char* argv[]) {
  flag_ctx* ctx = flag_context_init(20, 3);

  // Define flags with intuitive names and descriptions
  int integer_flag = 0;
  size_t size_t_flag = 0;
  int8_t int8_flag = 0;
  int16_t int16_flag = 0;
  int32_t int32_flag = 0;
  int64_t int64_flag = 0;
  unsigned int uint_flag = 0;
  uint8_t uint8_flag = 0;
  uint16_t uint16_flag = 0;
  uint32_t uint32_flag = 0;
  uint64_t uint64_flag = 0;
  uintptr_t uintptr_flag = 0;
  float float32_flag = 0.0;
  double float64_flag = 0.0;
  const char* string_flag = "";

  flag_validator int_validator = {
    .validator = validate_int,
    .error_message = "Must be between 0 and 10",
  };

  flag_add_with_validator(ctx, "int", &integer_flag, FLAG_INT,
                          "An integer flag", false, &int_validator);

  flag_add(ctx, "size_t", &size_t_flag, FLAG_SIZE_T, "A size_t flag", false);
  flag_add(ctx, "int8", &int8_flag, FLAG_INT8, "An int8_t flag", false);
  flag_add(ctx, "int16", &int16_flag, FLAG_INT16, "An int16_t flag", false);
  flag_add(ctx, "int32", &int32_flag, FLAG_INT32, "An int32_t flag", false);
  flag_add(ctx, "int64", &int64_flag, FLAG_INT64, "An int64_t flag", false);
  flag_add(ctx, "uint", &uint_flag, FLAG_UINT, "An unsigned int flag", false);
  flag_add(ctx, "uint8", &uint8_flag, FLAG_UINT8, "A uint8_t flag", false);
  flag_add(ctx, "uint16", &uint16_flag, FLAG_UINT16, "A uint16_t flag", false);
  flag_add(ctx, "uint32", &uint32_flag, FLAG_UINT32, "A uint32_t flag", false);
  flag_add(ctx, "uint64", &uint64_flag, FLAG_UINT64, "A uint64_t flag", false);
  flag_add(ctx, "uintptr", &uintptr_flag, FLAG_UINTPTR, "A uintptr_t flag",
           false);
  flag_add(ctx, "float32", &float32_flag, FLAG_FLOAT, "A float32 flag", false);
  flag_add(ctx, "float64", &float64_flag, FLAG_DOUBLE, "A float64 flag", false);
  flag_add(ctx, "string", &string_flag, FLAG_STRING, "A string flag", false);

  int count = 0;
  bool verbose = true;

  subcommand* subcmd1 =
    flag_add_subcommand(ctx, "subcommand", "print hello", handle_subcommand, 2);
  subcommand_add_flag(subcmd1, "verbose", &verbose, FLAG_BOOL, "Verbose output",
                      false, NULL);

  subcommand_add_flag(subcmd1, "count", &count, FLAG_INT,
                      "The number of times to print", false, NULL);

  subcommand* subcmdGreet =
    flag_add_subcommand(ctx, "greet", "Greets the user", handle_greet, 1);

  char* name = "Guest";
  subcommand_add_flag(subcmdGreet, "name", &name, FLAG_STRING,
                      "The name of the user", false, NULL);

  // Parse flags
  subcommand* cmd = parse_flags(ctx, argc, argv);

  // Print flag values
  printf("Parsed flag values:\n");
  printf("int: %d\n", integer_flag);
  printf("size_t: %zu\n", size_t_flag);
  printf("int8: %d\n", int8_flag);
  printf("int16: %d\n", int16_flag);
  printf("int32: %d\n", int32_flag);
  printf("int64: %ld\n", int64_flag);
  printf("uint: %u\n", uint_flag);
  printf("uint8: %u\n", uint8_flag);
  printf("uint16: %u\n", uint16_flag);
  printf("uint32: %u\n", uint32_flag);
  printf("uint64: %lu\n", uint64_flag);
  printf("uintptr: %lu\n", uintptr_flag);
  printf("float32: %f\n", float32_flag);
  printf("float64: %lf\n", float64_flag);
  printf("string: %s\n", string_flag);

  // Run the matching subcommand. Returned from parse_args
  if (cmd) {
    subcommand_call(cmd, ctx);
  }

  // Clean up allocated memory
  flag_destroy_context(ctx);

  return 0;
}

```

### Compile and run the example

```bash
chmod +x run.sh
./run.sh
```

### License

MIT License
