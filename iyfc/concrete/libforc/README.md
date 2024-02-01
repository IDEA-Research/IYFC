cargo build 


cbindgen --config cbindgen.toml --crate libforc --output concrete_header.h

gcc -o concrete_test_main concrete_test_main.cpp target/debug/libconcrete_header.a -lpthread -ldl -lm

