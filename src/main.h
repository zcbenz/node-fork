#ifndef MAIN_H
#define MAIN_H

#include <node.h>
#include <v8.h>
using namespace v8;

#define THROW_BAD_ARGS \
    ThrowException(Exception::TypeError(String::New("Bad argument")))

#endif /* end of MAIN_H */
