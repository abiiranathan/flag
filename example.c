#include "flag.h"

static int integer_flag = 0;
static size_t size_t_flag = 0;
static int8_t int8_flag = 0;
static int16_t int16_flag = 0;
static int32_t int32_flag = 0;
static int64_t int64_flag = 0;
static unsigned int uint_flag = 0;
static uint8_t uint8_flag = 0;
static uint16_t uint16_flag = 0;
static uint32_t uint32_flag = 0;
static uint64_t uint64_flag = 0;
static uintptr_t uintptr_flag = 0;
static float float32_flag = 0.0;
static double float64_flag = 0.0;
static const char* string_flag = "";

bool validate_int(const void* value) {
  int* int_value = (int*)value;
  return (*int_value >= 0 && *int_value <= 10);
}

void handler(FlagArgs args) {
  int count = *(int*)FlagValue(args.flags, args.num_flags, "count");
  bool verbose = *(bool*)FlagValue(args.flags, args.num_flags, "verbose");

  printf("count=%d verbose=%d\n", count, verbose);

  // ctx will provide you with access to global flags
  double float64 = *(double*)FlagValueCtx(args.ctx, "float64");
  printf("float64 value in callback: %lf\n", float64);
}

void handle_greet(FlagArgs args) {
  const char* name = *(const char**)FlagValue(args.flags, args.num_flags, "name");
  printf("Hello, %s!\n", name);
}

void registerSubcommands(flag_ctx* ctx) {
  int count = 0;
  bool verbose = true;
  subcommand *cmd1, *cmd2;

  cmd1 = AddSubCmd(ctx, .name = "print", .desc = "print hello", .handler = handler, .capacity = 2);
  AddSubCmdFlag(cmd1, .name = "verbose", .value = &verbose, .type = FLAG_BOOL,
                .desc = "Verbose output", );

  // Add flags to subcommand
  flag* fcount;
  fcount = AddSubCmdFlag(cmd1, .name = "count", .value = &count, .type = FLAG_INT,
                         .desc = "The number of times to print hello", );

  SetValidator(fcount, validate_int, "count must be between 0 and 10");

  // Second subcommand
  cmd2 = AddSubCmd(ctx, .name = "greet", .desc = "Greets the user", .handler = handle_greet,
                   .capacity = 1);

  char* name = "Guest";
  AddSubCmdFlag(cmd2, .name = "name", .value = &name, .type = FLAG_STRING,
                .desc = "The name of the user to greet", );
}

int main(int argc, char* argv[]) {
  flag_ctx* ctx = CreateFlagContext();

  AddFlag(ctx, .name = "int", .value = &integer_flag, .type = FLAG_INT, "An integer flag");
  AddFlag(ctx, .name = "size_t", .value = &size_t_flag, .type = FLAG_SIZE_T, "A size_t flag");
  AddFlag(ctx, .name = "int8", .value = &int8_flag, .type = FLAG_INT8, "An int8_t flag");
  AddFlag(ctx, .name = "int16", .value = &int16_flag, .type = FLAG_INT16, "An int16_t flag");
  AddFlag(ctx, .name = "int32", .value = &int32_flag, .type = FLAG_INT32, "An int32_t flag");
  AddFlag(ctx, .name = "int64", .value = &int64_flag, .type = FLAG_INT64, "An int64_t flag");
  AddFlag(ctx, .name = "uint", .value = &uint_flag, .type = FLAG_UINT, "An unsigned int flag");
  AddFlag(ctx, .name = "uint8", .value = &uint8_flag, .type = FLAG_UINT8, "A uint8_t flag");
  AddFlag(ctx, .name = "uint16", .value = &uint16_flag, .type = FLAG_UINT16, "A uint16_t flag");
  AddFlag(ctx, .name = "uint32", .value = &uint32_flag, .type = FLAG_UINT32, "A uint32_t flag");
  AddFlag(ctx, .name = "uint64", .value = &uint64_flag, .type = FLAG_UINT64, "A uint64_t flag");
  AddFlag(ctx, .name = "uintptr", .value = &uintptr_flag, .type = FLAG_UINTPTR, "A uintptr_t flag");
  AddFlag(ctx, .name = "float32", .value = &float32_flag, .type = FLAG_FLOAT, "A float32 flag");
  AddFlag(ctx, .name = "float64", .value = &float64_flag, .type = FLAG_DOUBLE, "A float64 flag");
  AddFlag(ctx, .name = "string", .value = &string_flag, .type = FLAG_STRING, "A string flag");

  registerSubcommands(ctx);

  // Parse flags
  subcommand* cmd = ParseFlags(ctx, argc, argv);

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
    InvokeSubCmd(cmd, ctx);
  }

  // Clean up allocated memory
  DestroyFlagContext(ctx);

  return 0;
}