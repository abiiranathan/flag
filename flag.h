/* flag implements a command line flag parser.

The flag package accepts command line arguments in the form of -flag value
or --arg value.

The flag package supports the following types:
bool, int, float32, float64, int8_t, int16_t, int32_t, int64_t, uint8_t,
uint16_t, uint32_t, uint64_t, uintptr_t, size_t, unsigned int, and char *

The package supports a shorthand notation where a
flag may be specified with just a single dash (-) instead of the full --.
Warning: The package does not support specifying a flag multiple times. In
this case, the last value will overwrite the previous ones.

The package also supports subcommands. Subcommands are specified by
providing a subcommand struct to the parse_flags function. The subcommand
struct contains a name, description, callback and flags. Only one subcommand
may be specified at a time and is assumed to be the 2nd argument. i.e argv[1]
*/

#ifndef __FLAG_H__
#define __FLAG_H__

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Supported flag types
typedef enum {
  FLAG_BOOL,     // bool
  FLAG_INT,      // int
  FLAG_SIZE_T,   // size_t
  FLAG_INT8,     // int8_t
  FLAG_INT16,    // int16_t
  FLAG_INT32,    // int32_t
  FLAG_INT64,    // int64_t
  FLAG_UINT,     // unsigned int
  FLAG_UINT8,    // uint8_t
  FLAG_UINT16,   // uint16_t
  FLAG_UINT32,   // uint32_t
  FLAG_UINT64,   // uint64_t
  FLAG_UINTPTR,  // uintptr_t
  FLAG_FLOAT,    // float
  FLAG_DOUBLE,   // double
  FLAG_STRING,   // char *
} flag_type;

// Flag validator contains a callback function to validate a flag
// and an error message to print if validation fails
typedef struct flag_validator {
  bool (*validator)(const void* value);  // optional callback to validate flag
  const char* error_message;             // error message to print if validation fails
} flag_validator;

// Create a struct to hold flag information
typedef struct flag {
  char* name;                      // flag name. Also used for flag lookup
  void* value;                     // Value stored in the flag.
  flag_type type;                  // Flag Type enum.
  char* description;               // Flag description.
  flag_validator* flag_validator;  // Optional validator for this flag.
} flag;

// Create a flag context to store global flags
typedef struct flag_ctx {
  flag* flags;           // flags in global context.
  size_t flag_capacity;  // maximum number of flags allowed in flag_ctx.
  int num_flags;         // Number of flags stored.
} flag_ctx;

// subcommand handler arguments.
typedef struct FlagArgs {
  flag* flags;    // subcommand flags
  int num_flags;  // number of flags
  flag_ctx* ctx;  // global ctx(to access other global flags)
} FlagArgs;

// Subcommand struct.
typedef struct subcommand {
  const char* name;         // name of the subcommand.
  const char* description;  // usage description.

  // optional callback. Called automatically with flags, num_flags and global flag context.
  // when done parsing it's flags.
  void (*callback)(FlagArgs args);
  flag* flags;    // flags for this subcommand.
  int num_flags;  // number of flags for this subcommand.
} subcommand;

// Create a global flag context. Must be freed with flag_destroy_context
flag_ctx flag_create_context(size_t flag_capacity);

// Get string representation of flag type. returns "unknown" if type is not valid.
const char* flag_type_string(flag_type type);

// Add a global flag to the flag context
void flag_add(flag_ctx* ctx, const char* name, void* value, flag_type type,
              const char* description);

// Add a flag to the flag context with a validator.
// The validator is a callback function that takes a void pointer to the flag
// value
void flag_add_with_validator(flag_ctx* ctx, const char* name, void* value, flag_type type,
                             const char* description, flag_validator* validator);

// Free memory used by flag_ctx flags.
void flag_destroy_context(flag_ctx* ctx);

// Parse global flags and subcommands, performing validation,
// overflow checks and calling subcommand callbacks.
// Optional subcommand array (subcommand *) may be passed to parse subcommands
// followed the number of subcommands(int).
//
// Returns a ptr to the matching subcommand(or NULL if no subcommands) that can invoked with
// the function subcommand_call.
subcommand* parse_flags(flag_ctx* ctx, int argc, char* argv[], subcommand* subcommands,
                        size_t num_subcommands);

// Extract value of the flag by name given an array of flags.
// Return a pointer to the value or NULL if not found.
void* flag_value(flag* flags, int num_flags, const char* name);

// Get value from global flag context.
void* flag_value_ctx(flag_ctx* ctx, const char* name);


// Invoke the subcommand callback.
void subcommand_call(subcommand* subcmd, flag_ctx* ctx);

// Print help message for flags in flag context.
// Does not exit the program automatically.
void print_help(flag_ctx* ctx, char** argv);


// FLAG IMLPEMENTATION
#ifdef FLAG_IMPLEMENTATION

// Initialize a flag context
flag_ctx flag_create_context(size_t flag_capacity) {
  flag_ctx ctx = {0};
  ctx.flags = (flag*)malloc(sizeof(flag) * flag_capacity);
  if (ctx.flags == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for flags\n");
    exit(EXIT_FAILURE);
  }

  ctx.flag_capacity = flag_capacity;
  return ctx;
}

// Add a flag to the flag context
void flag_add(flag_ctx* ctx, const char* name, void* value, flag_type type,
              const char* description) {
  // check for enough capacity
  if (ctx->num_flags >= ctx->flag_capacity) {
    fprintf(stderr, "Error: Not enough capacity to add flag %s\n", name);
    exit(EXIT_FAILURE);
  }

  ctx->flags[ctx->num_flags].name = strdup(name);
  ctx->flags[ctx->num_flags].value = value;
  ctx->flags[ctx->num_flags].type = type;
  ctx->flags[ctx->num_flags].description = strdup(description);
  ctx->num_flags++;
}

void flag_add_with_validator(flag_ctx* ctx, const char* name, void* value, flag_type type,
                             const char* description, flag_validator* validator) {
  flag_add(ctx, name, value, type, description);
  ctx->flags[ctx->num_flags - 1].flag_validator = validator;
}

// Function to check if a string is a valid integer
bool is_valid_integer(const char* str) {
  if (str == NULL || str[0] == '\0') {
    return false;
  }

  // Check for optional sign
  if (str[0] == '-' || str[0] == '+') {
    str++;
  }

  // Check for empty string after removing sign
  if (str[0] == '\0') {
    return false;
  }

  // Check if the remaining characters are digits
  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit((unsigned char)str[i])) {
      return false;
    }
  }

  return true;
}

// Print an error message and exit
void fatalf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

void validate_integer(flag* flags, const char** argv, int i, int j) {
  if (!is_valid_integer(argv[i])) {
    fprintf(stderr, "Error: Invalid integer value for flag %s\n", flags[j].name);
    exit(EXIT_FAILURE);
  }
}

// Function to find a command by name
subcommand* find_subcommand(subcommand* subcmds, int num_commands, const char* name) {
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(subcmds[i].name, name) == 0) {
      return &subcmds[i];
    }
  }
  return NULL;
}

flag* find_flag(flag* flags, int num_flags, char** argv, int index) {
  for (int i = 0; i < num_flags; i++) {
    if (strcmp(flags[i].name, argv[index]) == 0) {
      return &flags[i];
    }
  }
  return NULL;
}

void* flag_value(flag* flags, int num_flags, const char* name) {
  for (int i = 0; i < num_flags; i++) {
    if (strcmp(flags[i].name, name) == 0) {
      return flags[i].value;
    }
  }
  return NULL;
}

void* flag_value_ctx(flag_ctx* ctx, const char* name) {
  for (int i = 0; i < ctx->num_flags; i++) {
    if (strcmp(ctx->flags[i].name, name) == 0) {
      return ctx->flags[i].value;
    }
  }
  return NULL;
}

// Invoke the subcommand callback.
void subcommand_call(subcommand* subcmd, flag_ctx* ctx) {
  if (subcmd == NULL || ctx == NULL)
    return;

  // Once we are done. We call the subcommand callback.
  if (subcmd->callback != NULL) {
    subcmd->callback(
      (FlagArgs){.flags = subcmd->flags, .num_flags = subcmd->num_flags, .ctx = ctx});
  }
}

void parse_subcommand_flag(flag* flag, char* arg) {
  errno = 0;  // Reset errno before calling atoi/atoll/atof/strtod etc.

  switch (flag->type) {
    case FLAG_BOOL: {
      // If no value is specified, consider it as true
      if ((strcasecmp(arg, "true") == 0)) {
        *((bool*)flag->value) = true;
      } else if (strcasecmp(arg, "false") == 0) {
        *((bool*)flag->value) = false;
      } else {
        fatalf("Error: Invalid boolean value %s for flag %s\n", arg, flag->name);
      }
    } break;
    case FLAG_INT: {
      long long_value = strtol(arg, NULL, 10);
      if (errno == ERANGE) {
        fatalf("Error: Integer overflow for flag %s\n", flag->name);
      }

      *((int*)flag->value) = (int)long_value;
    } break;
    case FLAG_SIZE_T: {
      unsigned long long_size_t_value = strtoul(arg, NULL, 10);
      if (errno == ERANGE) {
        fatalf("Error: Size_t overflow for flag %s\n", flag->name);
      }
      *((size_t*)flag->value) = (size_t)long_size_t_value;
    } break;
    case FLAG_INT8: {
      intmax_t int_value = strtoimax(arg, NULL, 10);
      if (errno == ERANGE || int_value < INT8_MIN || int_value > INT8_MAX) {
        fatalf("Error: Int8 overflow or underflow for flag %s\n", flag->name);
      }
      *((int8_t*)flag->value) = (int8_t)int_value;
    } break;
    case FLAG_INT16: {
      intmax_t int_value = strtoimax(arg, NULL, 10);
      if (errno == ERANGE || int_value < INT16_MIN || int_value > INT16_MAX) {
        fatalf("Error: Int16 overflow or underflow for flag %s\n", flag->name);
      }
      *((int16_t*)flag->value) = (int16_t)int_value;
    } break;
    case FLAG_INT32: {
      intmax_t int_value = strtoimax(arg, NULL, 10);
      if (errno == ERANGE || int_value < INT32_MIN || int_value > INT32_MAX) {
        fatalf("Error: Int32 overflow or underflow for flag %s\n", flag->name);
      }
      *((int32_t*)flag->value) = (int32_t)int_value;
    } break;
    case FLAG_INT64: {
      intmax_t int_value = strtoimax(arg, NULL, 10);
      if (errno == ERANGE || int_value < INT64_MIN || int_value > INT64_MAX) {
        fatalf("Error: Int64 overflow or underflow for flag %s\n", flag->name);
      }
      *((int64_t*)flag->value) = (int64_t)int_value;
    } break;
    case FLAG_UINT8: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINT8_MAX) {
        fatalf("Error: Uint8 overflow for flag %s\n", flag->name);
      }
      *((uint8_t*)flag->value) = (uint8_t)uint_value;
    } break;
    case FLAG_UINT16: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINT16_MAX) {
        fatalf("Error: Uint16 overflow for flag %s\n", flag->name);
      }
      *((uint16_t*)flag->value) = (uint16_t)uint_value;
    } break;
    case FLAG_UINT32: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINT32_MAX) {
        fatalf("Error: Uint32 overflow for flag %s\n", flag->name);
      }
      *((uint32_t*)flag->value) = (uint32_t)uint_value;
    } break;
    case FLAG_UINT: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINT_MAX) {
        fatalf("Error: Uint overflow for flag %s\n", flag->name);
      }
      *((unsigned int*)flag->value) = (unsigned int)uint_value;
    } break;
    case FLAG_UINT64: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINT64_MAX) {
        fatalf("Error: Uint64 overflow for flag %s\n", flag->name);
      }
      *((uint64_t*)flag->value) = (uint64_t)uint_value;
    } break;
    case FLAG_UINTPTR: {
      uintmax_t uint_value = strtoumax(arg, NULL, 10);
      if (errno == ERANGE || uint_value > UINTPTR_MAX) {
        fatalf("Error: Uint64 overflow for flag %s\n", flag->name);
      }
      *((uintptr_t*)flag->value) = (uintptr_t)uint_value;
    } break;
    case FLAG_FLOAT: {
      float float_value = strtof(arg, NULL);
      if (errno == ERANGE) {
        fatalf("Error: float overflow for flag %s\n", flag->name);
      }
      *((float*)flag->value) = float_value;
    } break;
    case FLAG_DOUBLE: {
      double double_value = strtod(arg, NULL);
      if (errno == ERANGE) {
        fatalf("Error: double overflow for flag %s\n", flag->name);
      }
      *((double*)flag->value) = double_value;
    } break;
    case FLAG_STRING: {
      *((char**)flag->value) = strdup(arg);
    } break;
  }
}

void parse_flag_helper(flag* flags, int num_flags, int* iptr, int argc, char** argv) {
  int i = *iptr;
  // Handle both single-dash and double-dash flag notation
  const char* flag_name = (argv[i][1] == '-') ? &argv[i][2] : &argv[i][1];

  for (int j = 0; j < num_flags; j++) {
    if (strcmp(flag_name, flags[j].name) == 0) {
      i++;
      errno = 0;  // Reset errno before calling atoi/atoll/atof/strtod etc.
      switch (flags[j].type) {
        case FLAG_BOOL: {
          // If no value is specified, consider it as true
          if (i >= argc || (strcasecmp(argv[i], "true") == 0)) {
            *((bool*)flags[j].value) = true;
          } else if (strcasecmp(argv[i], "false") == 0) {
            *((bool*)flags[j].value) = false;
          } else {
            fatalf("Error: Invalid boolean value for flag %s\n", flags[j].name);
          }
        } break;
        case FLAG_INT: {
          validate_integer(flags, (const char**)argv, i, j);
          long long_value = strtol(argv[i], NULL, 10);
          if (errno == ERANGE) {
            fatalf("Error: Integer overflow for flag %s\n", flags[j].name);
          }

          *((int*)flags[j].value) = (int)long_value;
        } break;
        case FLAG_SIZE_T: {
          validate_integer(flags, (const char**)argv, i, j);

          unsigned long long_size_t_value = strtoul(argv[i], NULL, 10);
          if (errno == ERANGE) {
            fatalf("Error: Size_t overflow for flag %s\n", flags[j].name);
          }
          *((size_t*)flags[j].value) = (size_t)long_size_t_value;
        } break;
        case FLAG_INT8: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT8_MIN || int_value > INT8_MAX) {
            fatalf("Error: Int8 overflow or underflow for flag %s\n", flags[j].name);
          }
          *((int8_t*)flags[j].value) = (int8_t)int_value;
        } break;
        case FLAG_INT16: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT16_MIN || int_value > INT16_MAX) {
            fatalf("Error: Int16 overflow or underflow for flag %s\n", flags[j].name);
          }
          *((int16_t*)flags[j].value) = (int16_t)int_value;
        } break;
        case FLAG_INT32: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT32_MIN || int_value > INT32_MAX) {
            fatalf("Error: Int32 overflow or underflow for flag %s\n", flags[j].name);
          }
          *((int32_t*)flags[j].value) = (int32_t)int_value;
        } break;
        case FLAG_INT64: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT64_MIN || int_value > INT64_MAX) {
            fatalf("Error: Int64 overflow or underflow for flag %s\n", flags[j].name);
          }
          *((int64_t*)flags[j].value) = (int64_t)int_value;
        } break;
        case FLAG_UINT8: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINT8_MAX) {
            fatalf("Error: Uint8 overflow for flag %s\n", flags[j].name);
          }
          *((uint8_t*)flags[j].value) = (uint8_t)uint_value;
        } break;
        case FLAG_UINT16: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINT16_MAX) {
            fatalf("Error: Uint16 overflow for flag %s\n", flags[j].name);
          }
          *((uint16_t*)flags[j].value) = (uint16_t)uint_value;
        } break;
        case FLAG_UINT32: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINT32_MAX) {
            fatalf("Error: Uint32 overflow for flag %s\n", flags[j].name);
          }
          *((uint32_t*)flags[j].value) = (uint32_t)uint_value;
        } break;
        case FLAG_UINT: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINT_MAX) {
            fatalf("Error: Uint overflow for flag %s\n", flags[j].name);
          }
          *((unsigned int*)flags[j].value) = (unsigned int)uint_value;
        } break;
        case FLAG_UINT64: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINT64_MAX) {
            fatalf("Error: Uint64 overflow for flag %s\n", flags[j].name);
          }
          *((uint64_t*)flags[j].value) = (uint64_t)uint_value;
        } break;
        case FLAG_UINTPTR: {
          validate_integer(flags, (const char**)argv, i, j);

          uintmax_t uint_value = strtoumax(argv[i], NULL, 10);
          if (errno == ERANGE || uint_value > UINTPTR_MAX) {
            fatalf("Error: Uint64 overflow for flag %s\n", flags[j].name);
          }
          *((uintptr_t*)flags[j].value) = (uintptr_t)uint_value;
        } break;
        case FLAG_FLOAT: {
          float float_value = strtof(argv[i], NULL);
          if (errno == ERANGE) {
            fatalf("Error: float overflow for flag %s\n", flags[j].name);
          }
          *((float*)flags[j].value) = float_value;
        } break;
        case FLAG_DOUBLE: {
          double double_value = strtod(argv[i], NULL);
          if (errno == ERANGE) {
            fatalf("Error: double overflow for flag %s\n", flags[j].name);
          }
          *((double*)flags[j].value) = double_value;
        } break;
        case FLAG_STRING: {
          *((char**)flags[j].value) = strdup(argv[i]);
        } break;
      }

      // If a validator is specified, call it
      if (flags[j].flag_validator != NULL && flags[j].flag_validator->validator != NULL) {
        if (!flags[j].flag_validator->validator(flags[j].value)) {
          fprintf(stderr, "Error: Invalid value for flag %s: %s\n", flags[j].name,
                  flags[j].flag_validator->error_message == NULL
                    ? "Invalid value"
                    : flags[j].flag_validator->error_message);

          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

// Parse command line arguments and set flag values
subcommand* parse_flags(flag_ctx* ctx, int argc, char* argv[], subcommand* subcommands,
                        size_t num_subcommands) {
  if (argc < 2) {
    return NULL;  // no subcommands or arguments
  }

  // Loop over all arguments in 2 passes.
  // The first pass handles global flags.
  // The second pass handles subcommands.
  int subcmdIndex = -1;
  subcommand* subcmd = NULL;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      const char* flag_name = (argv[i][1] == '-') ? &argv[i][2] : &argv[i][1];

      // handle help request.
      if (strcmp(flag_name, "help") == 0) {
        print_help(ctx, argv);
        exit(EXIT_SUCCESS);
      }

      parse_flag_helper(ctx->flags, ctx->num_flags, &i, argc, argv);
    } else {
      // check if this is a subcommand
      if (!subcommands || num_subcommands == 0) {
        continue;
      }

      // Find subcommand matching current argument.
      subcmd = find_subcommand(subcommands, num_subcommands, &argv[i][0]);
      if (subcmd) {
        subcmdIndex = i;
        break;  // stop processing global flags.
      }
    }
  }


  // Handle subcommand if it was found.
  if (!subcmd) {
    return NULL;
  }

  ++subcmdIndex;  // start after subcommand and continue up to argc
  while (subcmdIndex < argc) {
    char* arg = argv[subcmdIndex][0] == '-' ? &argv[subcmdIndex][1] : &argv[subcmdIndex][0];
    flag* flag = NULL;

    // find flag index matching next flag.
    for (size_t j = 0; j < subcmd->num_flags; j++) {
      if (strcmp(subcmd->flags[j].name, arg) == 0) {
        flag = &subcmd->flags[j];
        break;
      }
    }

    ++subcmdIndex;  // increment i to process next argument(value).
    if (flag != NULL) {
      parse_subcommand_flag(flag, argv[subcmdIndex]);
    }
  }
  return subcmd;
}

// Print help message for available flags
void print_help(flag_ctx* ctx, char** argv) {
  int max_name_length = 0;
  int max_type_length = 0;
  for (int i = 0; i < ctx->num_flags; i++) {
    int name_length = strlen(ctx->flags[i].name);
    if (name_length > max_name_length) {
      max_name_length = name_length;
    }

    int type_length = strlen(flag_type_string(ctx->flags[i].type));
    if (type_length > max_type_length) {
      max_type_length = type_length;
    }
  }

  // Print help message for available flags with aligned text
  printf("%s\n", argv[0]);
  printf("Available flags:\n");
  for (int i = 0; i < ctx->num_flags; i++) {
    printf("  -%-*s --%s <%s>: %s\n\n", max_name_length, ctx->flags[i].name, ctx->flags[i].name,
           flag_type_string(ctx->flags[i].type), ctx->flags[i].description);
  }
}

// Convert flag type to a string for printing
const char* flag_type_string(flag_type type) {
  switch (type) {
    case FLAG_BOOL:
      return "bool";
    case FLAG_INT:
      return "int";
    case FLAG_SIZE_T:
      return "size_t";
    case FLAG_INT8:
      return "int8_t";
    case FLAG_INT16:
      return "int16_t";
    case FLAG_INT32:
      return "int32_t";
    case FLAG_INT64:
      return "int64_t";
    case FLAG_UINT:
      return "unsigned int";
    case FLAG_UINT8:
      return "uint8_t";
    case FLAG_UINT16:
      return "uint16_t";
    case FLAG_UINT32:
      return "uint32_t";
    case FLAG_UINT64:
      return "uint64_t";
    case FLAG_UINTPTR:
      return "uintptr_t";
    case FLAG_FLOAT:
      return "float";
    case FLAG_DOUBLE:
      return "double";
    case FLAG_STRING:
      return "char *";
    default:
      return "unknown";
  }
}

void flag_destroy_context(flag_ctx* ctx) {
  // Free global flags
  for (int i = 0; i < ctx->num_flags; i++) {
    free(ctx->flags[i].name);
    free(ctx->flags[i].description);
  }
  free(ctx->flags);
}

#endif /* FLAG_IMPLEMENTATION */

#endif /* __FLAG_H__ */
