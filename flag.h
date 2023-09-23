/**
 * @file flag.h
 * @brief Header file containing declarations for a command-line flag parsing library.
 * 
 * This file contains declarations for a command-line flag parsing library. It defines the
 * structures and functions used to create and manage flags and subcommands, parse command-line
 * arguments, and perform validation on flag values.
 * 
 * The library supports a variety of flag types, including bool, int, size_t, int8_t, int16_t,
 * int32_t, int64_t, unsigned int, uint8_t, uint16_t, uint32_t, uint64_t, uintptr_t, float,
 * double, and char *. It also allows for the addition of custom validators to validate flag
 * values.
 * 
 * @author Dr. Abiira Nathan
 */

#ifndef __FLAG_H__
#define __FLAG_H__

// This macro ensures fdopen strdup are available
#define _DEFAULT_SOURCE 1

#include <assert.h>
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


#define f_assert(bool_expr, format, ...)                                       \
  if (!(bool_expr)) {                                                          \
    fprintf(stderr, format, ##__VA_ARGS__);                                    \
    assert((bool_expr));                                                       \
  }

#define f_assert_not_null(ptr, format, ...)                                    \
  f_assert((ptr) != NULL, format, ##__VA_ARGS__)

#ifndef MAX_NAME
#define MAX_NAME 64  // Maximum length of flag or subcommand name
#endif

#ifndef MAX_DESCRIPTION
#define MAX_DESCRIPTION 256  // Maximum length of flag or subcommand description
#endif

#ifndef MAX_GLOBAL_FLAGS
#define MAX_GLOBAL_FLAGS 24  // Maximum number of global flags
#endif

#ifndef MAX_SUBCOMMANDS
#define MAX_SUBCOMMANDS 10  // Maximum number of subcommands
#endif

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
  const char* error_message;  // error message to print if validation fails
} flag_validator;

// Create a struct to hold flag information
typedef struct flag {
  char name[MAX_NAME];                // flag name. Also used for flag lookup
  void* value;                        // Value stored in the flag.
  flag_type type;                     // Flag Type enum.
  char description[MAX_DESCRIPTION];  // Flag description.
  bool required;                      // This flag must be provided
  flag_validator* flag_validator;     // Optional validator for this flag.
} flag;

// subcommand handler arguments.
typedef struct FlagArgs {
  flag* flags;           // subcommand flags
  int num_flags;         // number of flags
  struct flag_ctx* ctx;  // global ctx(to access other global flags)
} FlagArgs;

// Subcommand struct.
typedef struct subcommand {
  char name[MAX_NAME];                // name of the subcommand.
  char description[MAX_DESCRIPTION];  // usage description.

  // optional callback. Called automatically with flags, num_flags and global flag context.
  // when done parsing it's flags.
  void (*callback)(FlagArgs args);
  struct flag* flags;  // flags for this subcommand.
  int num_flags;       // number of flags for this subcommand.
  int flag_capacity;   // Maximum flags
} subcommand;

// Create a flag context to store global flags
typedef struct flag_ctx {
  flag flags[MAX_GLOBAL_FLAGS];  // flags in global context.
  size_t num_flags;              // Number of global flags stored.

  subcommand* subcommands[MAX_SUBCOMMANDS];  // array of pointers to subcommands
  size_t num_subcommands;                    // number of subcommands
} flag_ctx;

// Create a global flag context. Must be freed with flag_destroy_context
flag_ctx* flag_context_init(void);

// Get string representation of flag type. returns "unknown" if type is not valid.
static const char* flag_type_string(flag_type type);

// Add a global flag to the flag context
void flag_add(flag_ctx* ctx, const char* name, void* value, flag_type type,
              const char* description, bool required);

// Add a flag to the flag context with a validator.
// The validator is a callback function that takes a void pointer to the flag
// value
void flag_add_with_validator(flag_ctx* ctx, const char* name, void* value,
                             flag_type type, const char* description,
                             bool required, flag_validator* validator);

subcommand* flag_add_subcommand(flag_ctx* ctx, const char* name,
                                const char* desc,
                                void (*handler)(FlagArgs args),
                                size_t flag_capacity);

void subcommand_add_flag(subcommand* subcmd, const char* name, void* value,
                         flag_type type, const char* description, bool required,
                         flag_validator* validator);

// Free memory used by flag_ctx flags.
void flag_destroy_context(flag_ctx* ctx);

// Parses global flags subcommands and their flags and performs validation.
subcommand* parse_flags(flag_ctx* ctx, int argc, char* argv[]);

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

#endif /* __FLAG_H__ */
