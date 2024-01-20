## Adding Commands to Memcached

### Description

Some additions have been made to Memcached, enhancing its capabilities to support multiplication and division commands. Now, you can perform operations like `mult x 4` and `div x 100` after setting the variable `x`. These changes are detailed as follows:

- In the file `proto_text.c`, replaced `short incr` in the function `static void process_arithmetic_command` and added code to recognize the tokens `mult` and `div.`
- In the `thread.c` file, replaced `short incr` in the function `enum delta_result_type add_delta`.
- In both `memcached.c` and `memcached.h`, replaced `const short incr` in the functions `enum delta_result_type do_add_delta` and `enum delta_result_type add_delta`. Also, added cases for `incr` to implement the functionality of multiplication and division.