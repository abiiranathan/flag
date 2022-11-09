// Override FLAG_CAPACITY macro
#define FLAG_CAPACITY 6
// define FLAG_IMPLEMENTATION to access the implementation
// before including flag.h
#define FLAG_IMPLEMENTATION
#include "flag.h"

int main(int argc, char **argv) {
  bool *help =
      flag_bool("help", false, "Print this help to stdout and exit with 0");
  char **addr = flag_str("addr", "localhost:8080", "The server address");
  long int *port = flag_int("port", 8080, "The port to listen on");
  float *minutes = flag_float("minutes", 60.0f, "The TTL");
  bool *use_tls = flag_bool("tls", false, "Use TLS");
  size_t *storage = flag_size("storage", 500, "The amount to store");

  flag_parse(argc, argv);

  if (*help) {
    flag_usage(stdout, argv[0]);
    exit(0);
  }

  printf("Addr: %s\n", *addr);
  printf("Port: %ld\n", *port);
  printf("Min: %lf\n", *minutes);
  printf("TLS Enabled: %d\n", *use_tls);
  printf("Bytes to store: %lu\n", *storage);
}
