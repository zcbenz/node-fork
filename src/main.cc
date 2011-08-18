#include "main.h"
#include "fork.h"
#include "sleep.h"
#include "async.h"
#include "future.h"

extern "C" {

static void init (Handle<Object> target)
{
    HandleScope scope;

    NODE_SET_METHOD(target, "fork", Fork::Fork);
    NODE_SET_METHOD(target, "sleep", Fork::Sleep);
    NODE_SET_METHOD(target, "async", Fork::Async);
    NODE_SET_METHOD(target, "future", Fork::Future);
}

NODE_MODULE (fork, init);

}
