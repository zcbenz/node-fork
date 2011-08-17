#include "main.h"
#include "fork.h"

extern "C" {

static void init (Handle<Object> target)
{
    HandleScope scope;

    NODE_SET_METHOD(target, "fork", Fork::Fork);
}

NODE_MODULE (fork, init);

}
