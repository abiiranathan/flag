// MIT License
//
// Copyright (c) 2022 Dr. Abiira Nathan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef FLAG_H__
#define FLAG_H__

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FLAG_CAPACITY
#define FLAG_CAPACITY 100
#endif

typedef enum { INTEGER, DOUBLE, FLOAT, STRING, BOOL, SIZE, UINT64 } FlagType;

typedef union {
  char *as_str;
  long int as_int;
  bool as_bool;
  float as_float;
  double as_double;
  size_t as_size;
  uint64_t as_uint64;
} FlagValue;

typedef struct {
  FlagType type;
  char *name;
  char *desc;
  FlagValue def;
  FlagValue value;
} Flag;

typedef enum {
  FLAG_NO_ERROR = 0,
  FLAG_ERROR_UNKNOWN,
  FLAG_ERROR_NO_VALUE,
  FLAG_ERROR_INVALID_NUMBER,
  FLAG_ERROR_INTEGER_OVERFLOW,
  FLAG_ERROR_INVALID_SIZE_SUFFIX,
  FLAG_ERROR_DOUBLE_OVERFLOW,
  FLAG_ERROR_FLOAT_OVERFLOW,
} Flag_Error;

#endif  // FLAG_H__

// must be defined to include implementation in the header file.
#ifdef FLAG_IMPLEMENTATION

typedef struct {
  Flag flags[FLAG_CAPACITY];
  size_t flags_count;

  Flag_Error flag_error;
  char *flag_error_name;

  int rest_argc;
  char **rest_argv;
} FlagContext;

// Initialize the flags global context
static FlagContext flag_global_context;

// Helper function to create all the flags.
// Must not be called directly.
Flag *flag_new(const char *name, FlagType type, const char *desc) {
  FlagContext *c = &flag_global_context;
  if (c->flags_count >= FLAG_CAPACITY) {
    fprintf(stderr,
            "exceeded the allowed flag capacity: %d, define a FLAG_CAPACITY "
            "macro to override this setting\n",
            FLAG_CAPACITY);
    exit(1);
  }

  // Access flag from the flags buffer and advance to flags count.
  Flag *flag = &c->flags[c->flags_count++];

  // zero out the flag.
  memset(flag, 0, sizeof(*flag));

  flag->type = type;
  // I don't care about const pointer correctness as these strings
  // are not changed( I swear!)
  flag->name = (char *)name;
  flag->desc = (char *)desc;
  return flag;
}

/// @brief Create a long int flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to long int.
long int *flag_int(char *name, long int def, char *desc) {
  Flag *flag = flag_new(name, INTEGER, desc);
  flag->value.as_int = def;
  flag->def.as_int = def;
  return &flag->value.as_int;
}

/// @brief Create a string flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to a char pointer.
char **flag_str(char *name, const char *def, char *desc) {
  Flag *flag = flag_new(name, STRING, desc);
  flag->value.as_str = (char *)def;
  flag->def.as_str = (char *)def;
  return &flag->value.as_str;
}

/// @brief Create a double flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to double.
double *flag_double(char *name, double def, char *desc) {
  Flag *flag = flag_new(name, DOUBLE, desc);
  flag->value.as_double = def;
  flag->def.as_double = def;
  return &flag->value.as_double;
}

/// @brief Create a float flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to float.
float *flag_float(char *name, float def, char *desc) {
  Flag *flag = flag_new(name, FLOAT, desc);
  flag->value.as_float = def;
  flag->def.as_float = def;
  return &flag->value.as_float;
}

/// @brief Create a bool flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to bool.
bool *flag_bool(char *name, bool def, char *desc) {
  Flag *flag = flag_new(name, BOOL, desc);
  flag->value.as_bool = def;
  flag->def.as_bool = def;
  return &flag->value.as_bool;
}

/// @brief Create a size_t flag. Used to parse file sizes.
/// e.g cli -size 300MB
/// If no units are provided size if interpreted as bytes.
// Other units are as 3M 4K 1G.
//  Large units are not yet supported
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to size_t.
size_t *flag_size(const char *name, size_t def, const char *desc) {
  Flag *flag = flag_new(name, SIZE, desc);
  flag->value.as_size = def;
  flag->def.as_size = def;
  return &flag->value.as_size;
}

/// @brief Create a uint64_t flag.
/// @param name The name of the command-line flag.
/// @param def  The default value.
/// @param desc The description.
/// @return Returns a pointer to uint64_t.
uint64_t *flag_uint64(const char *name, uint64_t def, const char *desc) {
  Flag *flag = flag_new(name, UINT64, desc);
  flag->value.as_uint64 = def;
  flag->def.as_uint64 = def;
  return &flag->value.as_uint64;
}

// Extract arguments from the command-line
static char *flag_shift_args(int *argc, char ***argv) {
  if (*argc < 0) {
    fprintf(stderr, "argc < 0\n");
    exit(1);
  }

  char *result = **argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

int flag_rest_argc(void) { return flag_global_context.rest_argc; }
char **flag_rest_argv(void) { return flag_global_context.rest_argv; }

// Print the flag options set.
void flag_print_options(FILE *stream) {
  FlagContext *c = &flag_global_context;
  for (size_t i = 0; i < c->flags_count; ++i) {
    Flag *flag = &c->flags[i];

    fprintf(stream, "    -%s\n", flag->name);
    fprintf(stream, "        %s\n", flag->desc);

    switch (c->flags[i].type) {
      case BOOL:
        if (flag->def.as_bool) {
          fprintf(stream, "        Default: %s\n",
                  flag->def.as_bool ? "true" : "false");
        }
        break;
      case INTEGER:
        fprintf(stream, "        Default: %ld\n", flag->def.as_int);
        break;
      case STRING:
        fprintf(stream, "        Default: %s\n", flag->def.as_str);
        break;
      case DOUBLE:
        if (flag->def.as_str) {
          fprintf(stream, "        Default: %lf\n", flag->def.as_double);
        }
        break;
      case FLOAT:
        if (flag->def.as_float) {
          fprintf(stream, "        Default: %f\n", flag->def.as_float);
        }
        break;
      case UINT64:
        fprintf(stream, "        Default: %" PRIu64 "\n", flag->def.as_uint64);
        break;
      case SIZE:
        fprintf(stream, "        Default: %zu\n", flag->def.as_size);
        break;
      default:
        fprintf(stderr, "unreachable option in switch statement\n");
        exit(2);
    }
  }
}

// Internal routine to parse the command-line arguments.
// Must be called after defining the flags.
bool __parse_flags(int argc, char **argv) {
  FlagContext *c = &flag_global_context;

  flag_shift_args(&argc, &argv);

  while (argc > 0) {
    char *flag = flag_shift_args(&argc, &argv);

    if (*flag != '-') {
      // NOTE: pushing flag back into args
      c->rest_argc = argc + 1;
      c->rest_argv = argv - 1;
      return true;
    }

    if (strcmp(flag, "--") == 0) {
      // NOTE: but if it's the terminator we don't need to push it back
      c->rest_argc = argc;
      c->rest_argv = argv;
      return true;
    }

    // NOTE: remove the dash
    flag += 1;
    bool found = false;

    for (size_t i = 0; i < c->flags_count; ++i) {
      if (strcmp(c->flags[i].name, flag) == 0) {
        if (argc == 0 && (c->flags[i].type != BOOL)) {
          c->flag_error = FLAG_ERROR_NO_VALUE;
          c->flag_error_name = flag;
          return false;
        }

        switch (c->flags[i].type) {
          case BOOL: {
            c->flags[i].value.as_bool = true;
          } break;

          case STRING: {
            char *arg = flag_shift_args(&argc, &argv);
            c->flags[i].value.as_str = arg;
          } break;

          case UINT64: {
            char *arg = flag_shift_args(&argc, &argv);
            char *endptr;
            unsigned long long int result = strtoull(arg, &endptr, 10);

            if (*endptr != '\0') {
              c->flag_error = FLAG_ERROR_INVALID_NUMBER;
              c->flag_error_name = flag;
              return false;
            }

            if (result == ULLONG_MAX && errno == ERANGE) {
              c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
              c->flag_error_name = flag;
              return false;
            }
            c->flags[i].value.as_uint64 = result;
          } break;

          case INTEGER: {
            char *endptr, *str;
            errno = 0;
            char *arg = flag_shift_args(&argc, &argv);
            str = arg;

            long int result = strtol(str, &endptr, 10);
            if (errno != 0) {
              if ((result == LONG_MAX || result == LONG_MIN) &&
                  errno == ERANGE) {
                c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
                c->flag_error_name = flag;
                return false;
              }

              if (endptr == str) {
                c->flag_error = FLAG_ERROR_NO_VALUE;
                c->flag_error_name = flag;
                return false;
              }

              c->flag_error = FLAG_ERROR_INVALID_NUMBER;
              c->flag_error_name = flag;
              return false;
            }

            c->flags[i].value.as_int = result;
          } break;

          case DOUBLE: {
            char *endptr;
            errno = 0;
            char *arg = flag_shift_args(&argc, &argv);
            double result = strtold(arg, &endptr);

            if (errno != 0) {
              if ((result == DBL_MAX || result == DBL_MIN) && errno == ERANGE) {
                c->flag_error = FLAG_ERROR_DOUBLE_OVERFLOW;
                c->flag_error_name = flag;
                return false;
              }

              c->flag_error = FLAG_ERROR_INVALID_NUMBER;
              c->flag_error_name = flag;
              return false;
            }

            c->flags[i].value.as_double = result;
          } break;

          case FLOAT: {
            char *endptr;
            errno = 0;
            char *arg = flag_shift_args(&argc, &argv);
            float result = strtof(arg, &endptr);

            if (errno != 0) {
              if ((result == FLT_MAX || result == FLT_MIN) && errno == ERANGE) {
                c->flag_error = FLAG_ERROR_FLOAT_OVERFLOW;
                c->flag_error_name = flag;
                return false;
              }
              c->flag_error = FLAG_ERROR_INVALID_NUMBER;
              c->flag_error_name = flag;
              return false;
            }
            c->flags[i].value.as_float = result;
          } break;

          case SIZE: {
            char *arg = flag_shift_args(&argc, &argv);
            char *endptr;

            unsigned long long int result = strtoull(arg, &endptr, 10);

            if (strcmp(endptr, "K") == 0) {
              result *= 1024;
            } else if (strcmp(endptr, "M") == 0) {
              result *= 1024 * 1024;
            } else if (strcmp(endptr, "G") == 0) {
              result *= 1024 * 1024 * 1024;
            } else if (strcmp(endptr, "") != 0) {
              c->flag_error = FLAG_ERROR_INVALID_SIZE_SUFFIX;
              c->flag_error_name = flag;
              return false;
            }

            if (result == ULLONG_MAX && errno == ERANGE) {
              c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
              c->flag_error_name = flag;
              return false;
            }
            c->flags[i].value.as_size = result;
          } break;
          default: {
            exit(2);
          }
        }

        found = true;
      }
    }

    if (!found) {
      c->flag_error = FLAG_ERROR_UNKNOWN;
      c->flag_error_name = flag;
      return false;
    }
  }

  c->rest_argc = argc;
  c->rest_argv = argv;
  return true;
}

// Prints the usage string to stream. executable is argv[0]
void flag_usage(FILE *stream, const char *executable) {
  fprintf(stream, "Usage: %s [OPTIONS] [--]\n", executable);
  fprintf(stream, "OPTIONS:\n");
  flag_print_options(stream);
}

// Prints the flag errors encountered while parsing
// the command-line arguments.
void flag_print_error(FILE *stream) {
  FlagContext *c = &flag_global_context;
  switch (c->flag_error) {
    case FLAG_NO_ERROR:
      fprintf(stream, "Operation Failed Unexpectedly! :)");
      break;
    case FLAG_ERROR_UNKNOWN:
      fprintf(stream, "ERROR: -%s: unknown flag\n", c->flag_error_name);
      break;
    case FLAG_ERROR_NO_VALUE:
      fprintf(stream, "ERROR: -%s: no value provided\n", c->flag_error_name);
      break;
    case FLAG_ERROR_INVALID_NUMBER:
      fprintf(stream, "ERROR: -%s: invalid number\n", c->flag_error_name);
      break;
    case FLAG_ERROR_INTEGER_OVERFLOW:
      fprintf(stream, "ERROR: -%s: integer overflow\n", c->flag_error_name);
      break;
    case FLAG_ERROR_INVALID_SIZE_SUFFIX:
      fprintf(stream, "ERROR: -%s: invalid size suffix\n", c->flag_error_name);
      break;
    case FLAG_ERROR_DOUBLE_OVERFLOW:
      fprintf(stream, "ERROR: -%s: double overflow\n", c->flag_error_name);
      break;
    case FLAG_ERROR_FLOAT_OVERFLOW:
      fprintf(stream, "ERROR: -%s: float overflow\n", c->flag_error_name);
      break;
    default:
      exit(69);
  }
}

// Parses the command-line arguments and updates the flags.
// Prints errors and usage if an error occurs
// and exits with a status code of 1.
void flag_parse(int argc, char **argv) {
  if (!__parse_flags(argc, argv)) {
    flag_print_error(stdout);
    printf("\n");
    flag_usage(stdout, argv[0]);
    exit(1);
  };
}

#endif