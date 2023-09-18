#!/bin/bash

rm -f ./flag

gcc -ggdb main.c -o flag

# ./flag greet -name "John Doe"

./flag -int 5 -size_t 100 -int8 127 -int16 16 -int32 32 -int64 64 -uint 4294967295 -uint8 255 -uint16 65535 -uint32 4294967295 -uint64 18446744073709551615 -uintptr 1234567890 -float32 3.14 -float64 2.718 -string "Hello, World!" subcommand -verbose true -count 30