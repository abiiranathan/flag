#include "flag.h"

extern void inline realdbgprintf(const char* SourceFilename, int SourceLineno,
                                 const char* CFormatString, ...);

static const char* flag_type_string(flag_type type);

// Initialize a flag context and add global help flag.
flag_ctx* flag_context_init(void) {
  flag_ctx* ctx = (flag_ctx*)malloc(sizeof(flag_ctx));
  f_assert(ctx != NULL, "[ERROR]: Unable to allocated memory for flag_ctx");

  ctx->num_flags = 0;
  ctx->num_subcommands = 0;

  memset(ctx->flags, 0, sizeof(flag) * MAX_GLOBAL_FLAGS);
  memset(ctx->subcommands, 0, sizeof(subcommand*) * MAX_SUBCOMMANDS);

  // Add help flag
  flag_add(ctx, "help", NULL, FLAG_BOOL, "Print help message and exit", false);
  return ctx;
}

void flag_destroy_context(flag_ctx* ctx) {
  if (ctx) {
    for (size_t i = 0; i < ctx->num_subcommands; i++) {
      free(ctx->subcommands[i]->flags);
      free(ctx->subcommands[i]);
    }
    free(ctx);
    ctx = NULL;
  }
}

// Add a flag to the flag context
void flag_add(flag_ctx* ctx, const char* name, void* value, flag_type type,
              const char* description, bool required) {
  // check for enough capacity
  f_assert(MAX_GLOBAL_FLAGS > ctx->num_flags,
           "[ERROR]: Not enough capacity in global flags to add flag: %s\n",
           name);


  // copy name
  strncpy(ctx->flags[ctx->num_flags].name, name, MAX_NAME - 1);
  // null terminate name
  ctx->flags[ctx->num_flags].name[MAX_NAME - 1] = '\0';

  ctx->flags[ctx->num_flags].value = value;
  ctx->flags[ctx->num_flags].type = type;

  strncpy(ctx->flags[ctx->num_flags].description, description,
          MAX_DESCRIPTION - 1);
  ctx->flags[ctx->num_flags].description[MAX_DESCRIPTION - 1] = '\0';

  ctx->flags[ctx->num_flags].required = required;
  ctx->flags[ctx->num_flags].flag_validator = NULL;
  ctx->num_flags++;
}

subcommand* flag_add_subcommand(flag_ctx* ctx, const char* name,
                                const char* desc,
                                void (*handler)(FlagArgs args),
                                size_t flag_capacity) {

  f_assert(
    MAX_SUBCOMMANDS > ctx->num_subcommands,
    "[ERROR]: Not enough capacity in subcommands to add subcommand: %s\n",
    name);

  f_assert(handler != NULL, "can not add subcommand with NULL handler");

  subcommand* subcmd = (subcommand*)malloc(sizeof(subcommand));
  f_assert(subcmd, "[ERROR]: Unable to allocate memory for subcommand");

  strncpy(subcmd->name, name, MAX_NAME - 1);
  subcmd->name[MAX_NAME - 1] = '\0';

  strncpy(subcmd->description, desc, MAX_DESCRIPTION - 1);
  subcmd->description[MAX_DESCRIPTION - 1] = '\0';


  subcmd->callback = handler;
  subcmd->flags = (flag*)malloc(flag_capacity * sizeof(flag));
  f_assert(subcmd->flags,
           "[ERROR]: Unable to allocate memory for subcommand flags");

  subcmd->num_flags = 0;
  subcmd->flag_capacity = flag_capacity;
  ctx->subcommands[ctx->num_subcommands] = subcmd;
  ctx->num_subcommands++;
  return subcmd;
}

void subcommand_add_flag(subcommand* subcmd, const char* name, void* value,
                         flag_type type, const char* description, bool required,
                         flag_validator* validator) {

  f_assert(subcmd->num_flags < subcmd->flag_capacity,
           "[ERROR]: Not enough capacity new subcommand(%s) flag: %s\n",
           subcmd->name, name);

  strncpy(subcmd->flags[subcmd->num_flags].name, name, MAX_NAME - 1);
  subcmd->flags[subcmd->num_flags].name[MAX_NAME - 1] = '\0';

  subcmd->flags[subcmd->num_flags].value = value;
  subcmd->flags[subcmd->num_flags].type = type;

  strncpy(subcmd->flags[subcmd->num_flags].description, description,
          MAX_DESCRIPTION - 1);
  subcmd->flags[subcmd->num_flags].description[MAX_DESCRIPTION - 1] = '\0';

  subcmd->flags[subcmd->num_flags].required = required;
  subcmd->flags[subcmd->num_flags].flag_validator = validator;
  subcmd->num_flags++;
}

void flag_add_with_validator(flag_ctx* ctx, const char* name, void* value,
                             flag_type type, const char* description,
                             bool required, flag_validator* validator) {
  flag_add(ctx, name, value, type, description, required);
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
    fprintf(stderr, "Error: Invalid integer value for flag %s\n",
            flags[j].name);
    exit(EXIT_FAILURE);
  }
}

// Function to find a command by name
subcommand* find_subcommand(subcommand** subcmds, int num_commands,
                            const char* name) {
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(subcmds[i]->name, name) == 0) {
      return subcmds[i];
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
  f_assert(subcmd != NULL, "subcommand can not be NULL");

  // Once we are done. We call the subcommand callback.
  FlagArgs args = {
    .flags = subcmd->flags,
    .num_flags = subcmd->num_flags,
    .ctx = ctx,
  };
  subcmd->callback(args);
}

void parse_subcommand_flag(flag* flag, char* arg) {
  errno = 0;  // Reset errno before conversion.

  switch (flag->type) {
    case FLAG_BOOL: {
      // If no value is specified, consider it as true
      if ((strcasecmp(arg, "true") == 0)) {
        *((bool*)flag->value) = true;
      } else if (strcasecmp(arg, "false") == 0) {
        *((bool*)flag->value) = false;
      } else {
        // assume it is a flag
        *((bool*)flag->value) = true;
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

void parse_flag_helper(flag* flags, int num_flags, int* iptr, int argc,
                       char** argv) {
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
            fatalf("Error: Int8 overflow or underflow for flag %s\n",
                   flags[j].name);
          }
          *((int8_t*)flags[j].value) = (int8_t)int_value;
        } break;
        case FLAG_INT16: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT16_MIN ||
              int_value > INT16_MAX) {
            fatalf("Error: Int16 overflow or underflow for flag %s\n",
                   flags[j].name);
          }
          *((int16_t*)flags[j].value) = (int16_t)int_value;
        } break;
        case FLAG_INT32: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT32_MIN ||
              int_value > INT32_MAX) {
            fatalf("Error: Int32 overflow or underflow for flag %s\n",
                   flags[j].name);
          }
          *((int32_t*)flags[j].value) = (int32_t)int_value;
        } break;
        case FLAG_INT64: {
          validate_integer(flags, (const char**)argv, i, j);

          intmax_t int_value = strtoimax(argv[i], NULL, 10);
          if (errno == ERANGE || int_value < INT64_MIN ||
              int_value > INT64_MAX) {
            fatalf("Error: Int64 overflow or underflow for flag %s\n",
                   flags[j].name);
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
            fatalf("Error: uintptr_t overflow for flag %s\n", flags[j].name);
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
      if (flags[j].flag_validator != NULL &&
          flags[j].flag_validator->validator != NULL) {
        if (!flags[j].flag_validator->validator(flags[j].value)) {
          fprintf(stderr, "Error: Invalid value for flag %s: %s\n",
                  flags[j].name,
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
subcommand* parse_flags(flag_ctx* ctx, int argc, char* argv[]) {
  if (argc < 2) {
    return NULL;  // no subcommands or arguments
  }

  // Loop over all arguments in 2 passes.
  // first pass  -> global flags.
  // second pass -> subcommands.
  int subcmdIndex = -1;
  subcommand* subcmd = NULL;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      const char* flag_name = (argv[i][1] == '-') ? &argv[i][2] : &argv[i][1];

      // handle help request.
      if (strcmp(flag_name, "help") == 0) {
        print_help(ctx, argv);
        flag_destroy_context(ctx);
        exit(EXIT_SUCCESS);
      }

      parse_flag_helper(ctx->flags, ctx->num_flags, &i, argc, argv);
    } else {
      // if we have no subcommands, continue to next argument.
      if (ctx->num_subcommands == 0) {
        continue;
      }

      // Find subcommand matching current argument.
      subcmd =
        find_subcommand(ctx->subcommands, ctx->num_subcommands, &argv[i][0]);
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

  // Create an array of processed flags. So we can validate required flags
  flag proccessed_flags[subcmd->num_flags];
  // zero the array of flag structs
  memset(proccessed_flags, 0, sizeof(flag) * subcmd->num_flags);

  ++subcmdIndex;  // start after subcommand and continue up to argc
  while (subcmdIndex < argc) {
    char* arg = argv[subcmdIndex][0] == '-' ? &argv[subcmdIndex][1]
                                            : &argv[subcmdIndex][0];
    flag* flag = NULL;

    // find flag index matching next flag.
    for (size_t j = 0; j < subcmd->num_flags; j++) {
      if (strcmp(subcmd->flags[j].name, arg) == 0) {
        flag = &subcmd->flags[j];
        proccessed_flags[j] = *flag;
        break;
      }
    }

    ++subcmdIndex;  // increment i to process next argument(value).

    if (flag != NULL) {
      if (arg == NULL) {
        fprintf(stderr, "Error: No value specified for flag %s\n", flag->name);
        print_help(ctx, argv);
        flag_destroy_context(ctx);
        exit(EXIT_SUCCESS);
      }

      parse_subcommand_flag(flag, argv[subcmdIndex]);
    }
  }

  // Post processing validation
  for (int i = 0; i < subcmd->num_flags; i++) {
    flag procFlag = proccessed_flags[i];
    flag actualFlag = subcmd->flags[i];

    if (actualFlag.required && procFlag.value == NULL) {
      fprintf(stderr, "\n[ERROR]: Flag %s is required\n\n", actualFlag.name);
      print_help(ctx, argv);
      flag_destroy_context(ctx);
      exit(EXIT_FAILURE);
    }
  }

  return subcmd;
}

static int maxNameLength(flag* flags, int n) {
  int max = 0;
  for (int i = 0; i < n; i++) {
    int len = strlen(flags[i].name);
    if (len > max) {
      max = len;
    }
  }
  return max;
}

static int maxTypeLength(flag* flags, int n) {
  int max = 0;
  for (int i = 0; i < n; i++) {
    int len = strlen(flag_type_string(flags[i].type));
    if (len > max) {
      max = len;
    }
  }
  return max;
}

// Print help message for available flags
void print_help(flag_ctx* ctx, char** argv) {
  int max_name_len_global = maxNameLength(ctx->flags, ctx->num_flags);

  // Print help message for available flags with aligned text
  printf("%s\n", argv[0]);
  printf("Global flags:\n");
  for (int i = 0; i < ctx->num_flags; i++) {
    printf("  -%-*s --%s(%s) <%s>: %s\n\n", max_name_len_global,
           ctx->flags[i].name, ctx->flags[i].name,
           ctx->flags[i].required ? "Required" : "Optional",
           flag_type_string(ctx->flags[i].type), ctx->flags[i].description);
  }


  int max_name_len_subcmd = 0;
  int max_type_len_subcmd = 0;

  for (int i = 0; i < ctx->num_subcommands; i++) {
    int name_len =
      maxNameLength(ctx->subcommands[i]->flags, ctx->subcommands[i]->num_flags);
    int type_len =
      maxTypeLength(ctx->subcommands[i]->flags, ctx->subcommands[i]->num_flags);

    if (name_len > max_name_len_subcmd) {
      max_name_len_subcmd = name_len;
    }

    if (type_len > max_type_len_subcmd) {
      max_type_len_subcmd = type_len;
    }
  }

  // print subcommands and their flags
  printf("Subcommands:\n");
  for (int i = 0; i < ctx->num_subcommands; i++) {
    printf("  %s: %s\n", ctx->subcommands[i]->name,
           ctx->subcommands[i]->description);

    for (int j = 0; j < ctx->subcommands[i]->num_flags; j++) {
      printf("    -%-*s --%s(%s) <%s>: %s\n", max_name_len_subcmd,
             ctx->subcommands[i]->flags[j].name,
             ctx->subcommands[i]->flags[j].name,
             ctx->subcommands[i]->flags[j].required ? "Required" : "Optional",
             flag_type_string(ctx->subcommands[i]->flags[j].type),
             ctx->subcommands[i]->flags[j].description);
    }
    printf("\n");
  }
  printf("\n");
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
