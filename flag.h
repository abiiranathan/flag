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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winitializer-overrides"

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

void inline realdbgprintf(const char* SourceFilename, int SourceLineno, const char* CFormatString,
                          ...) {
  va_list args;
  va_start(args, CFormatString);
  fprintf(stderr, "%s(%d): ", SourceFilename, SourceLineno);
  vfprintf(stderr, CFormatString, args);
  va_end(args);
  fprintf(stderr, "\n");
}

// Define dbgprintf
#define dbgprintf(...)                                                                             \
  (sizeof("" #__VA_ARGS__) > 1 ? realdbgprintf(__FILE__, __LINE__, __VA_ARGS__)                    \
                               : realdbgprintf(__FILE__, __LINE__, ""))


#ifdef NDEBUG
#define f_assert(expr, ...) ((void)0)
#else
#define f_assert(expr, ...)                                                                        \
  do {                                                                                             \
    if (!(expr)) {                                                                                 \
      realdbgprintf(__FILE__, __LINE__, __VA_ARGS__);                                              \
      assert((expr));                                                                              \
    }                                                                                              \
  } while (0)
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

// subcommand handler arguments passed to the subcommand handler.
// Get the value of a flag by name using flag_value.
// Get the value of a global flag by name using flag_value_ctx.
typedef struct FlagArgs {
  struct flag* flags;    // subcommand flags
  int num_flags;         // number of flags for subcommand
  struct flag_ctx* ctx;  // global ctx(to access other global flags)
} FlagArgs;


typedef struct flag_validator flag_validator;
typedef struct flag flag;
typedef struct subcommand subcommand;
typedef struct flag_ctx flag_ctx;
typedef bool (*validator)(const void* value);
typedef void (*flag_handler)(FlagArgs args);

// Parameter struct for flag creation.
typedef struct flag_params {
  const char* name;  // Name of flag
  void* value;       // Pointer to value
  flag_type type;    // Type of flag
  const char* desc;  // Description
  bool req;          // Required
} flag_params;

// Parameter struct for subcommand creation.
typedef struct subcmd_params {
  const char* name;      // Name of subcommand
  const char* desc;      // Description
  flag_handler handler;  // Subcommand handler
  size_t capacity;       // Maximum number of flags(capacity)
} subcmd_params;

// Create a global flag context. Must be freed with DestroyFlagContext.
flag_ctx* CreateFlagContext(void);

// Free memory used by flag_ctx flags.
void DestroyFlagContext(flag_ctx* ctx);

// Implementation files.
extern flag* _flag_add(flag_ctx* ctx, flag_params* params);
extern subcommand* _flag_add_subcommand(flag_ctx* ctx, subcmd_params* params);
extern flag* _subcommand_add_flag(subcommand* subcmd, flag_params* params);

// Add a flag to the flag context
#define AddFlag(ctx, ...)                                                                          \
  _flag_add(ctx, &(flag_params){.req = false, .type = FLAG_INT, __VA_ARGS__})

// Add subcommand to the flag context
#define AddSubCmd(ctx, ...) _flag_add_subcommand(ctx, &(subcmd_params){.capacity = 0, __VA_ARGS__})

#define AddSubCmdFlag(subcmd, ...)                                                                 \
  _subcommand_add_flag(subcmd, &(flag_params){.req = false, .type = FLAG_INT, __VA_ARGS__})

void SetValidator(flag* flag, validator val, const char* err_msg);

// Parses global flags subcommands and their flags and performs validation.
subcommand* ParseFlags(flag_ctx* ctx, int argc, char* argv[]);

// Extract value of the flag by name given an array of flags.
// Return a pointer to the value or NULL if not found.
void* FlagValue(flag* flags, int num_flags, const char* name);

// Get value from global flag context.
void* FlagValueCtx(flag_ctx* ctx, const char* name);

// Invoke the subcommand callback.
void InvokeSubCmd(subcommand* subcmd, flag_ctx* ctx);

// Print help message for flags in flag context.
// Does not exit the program automatically.
void PrintHelp(flag_ctx* ctx, char** argv);

#endif /* __FLAG_H__ */
