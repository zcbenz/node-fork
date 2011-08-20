# node-fork

 In short, `node-fork` makes a synchronous function asynchronous.

 In longer words, `node-fork` makes uses of `fork` system call to provide
 concurrency ability to the node.js, it can make a synchronous function
 really-asynchronous, thus completely get rid of the nested asynchronous
 calls.

 And if you're interested, the interface emulates C++0x's thread library.

## At a glance

### calculate_the_answer_to_LtUaE.js

```javascript
var sleep  = require("../fork").sleep;
var future = require("../fork").future;

function calculate_the_answer_to_LtUaE () {
    sleep (5);

    return 42;
}

var answer = future (calculate_the_answer_to_LtUaE);

console.log ("blablabla...");

console.log ("The answer to life, the universe and everything is",
             answer.get ());
```

### propagate.js

```javascript
var fork = require("../fork").fork;

for (var i = 0; i < 5; i++) {
    fork ();

    console.log (i);
};
```

### lots_of_websites.js

```javascript
// You will need node-curl to gain synchorous http ability
// at https://github.com/zcbenz/node-curl

var curl   = require("../node_curl");
var sleep  = require("../fork").sleep;
var async  = require("../fork").async;

console.log ("I'm visiting websites in 4 processes!");

async (function () {
    console.log ("sina");
    curl.get ("http://sina.com").end();
    console.log ("baidu");
    curl.get ("http://baidu.com").end();
    console.log ("weibo");
    curl.get ("http://weibo.com").end();
    console.log ("163");
    curl.get ("http://163.com").end();
    console.log ("sohu");
    curl.get ("http://sohu.com").end();
    console.log ("cnbeta");
    var res = curl.get ("http://cnbeta.com").end();

    return res.headers;
}, function (result) {
    console.log ("headers of cnbeta is:", result);
});

async (function () {
    console.log ("sohu");
    return curl.get ("http://sohu.com").end ().headers;
}, function (result) {
    console.log ("headers of sohu is:", result);
});

async (function () {
    console.log ("163");
    return curl.get ("http://163.com").end ().headers;
}, function (result) {
    console.log ("headers of 163 is:", result);
});

async (function () {
    console.log ("douban");
    curl.get ("http://douban.com").end ().headers;
    console.log ("taobao");
    return curl.get ("http://taobao.com").end ().headers;
}, function (result) {
    console.log ("headers of taobao is:", result);
});

// Just to block the main process
require("http").createServer(function (req, res) {
}).listen(9001);
```

## Why this when already have asynchronous calls in node

 In node days, if we want a chain of asynchronous calls, we would end up
 with something like this:

```javascript
fs.readFile ("a", function (err, data) {
    data += "blabla";
    fs.writeFile ("b", data, function (err) {
        fs.readFile ("c", function (err, data) {
            data += "blabla";
            fs.writeFile ("d", data, function (err) {
                console.log ("finally", data);
            });
        });
    });
});
```
 In one word, DISASTER.

 But with `node-fork`, we can use the synchronous functions just like the
 good old days:

```javascript
async (function () {
    fs.writeFileSync ("b", fs.readFileSync ("a") + "blabla");
    data = fs.readFileSync ("d") + "blabla";
    fs.writeFileSync ("d", data);

    return data;
}, function (data) {
    console.log ("finally", data);
});
```

## Build

 Just simply `make`.

## APIs

### future (call)

 `future()` will invoke the `call` function in a new process by calling
 `fork()`. The `call` function will have the exact state with the parent
 process.

 The return value of `future()` has a method `get()`, `get()` will return
 the return value of `call` function. If `call` has not ended when `get()`
 is called, the calling process of `get()` will wait until the `call`
 ends.

 Example

```javascript
function calculate_the_answer_to_LtUaE () {
    sleep (5);

    return 42;
}

var answer = future (calculate_the_answer_to_LtUaE);

console.log ("blablabla...");

console.log ("The answer to life, the universe and everything is",
             answer.get ());
```

### async (call, [back])

 `async()` will invoke the `call` function in a new process by calling
 `fork()`. The `call` function will have the exact state with the parent
 process.

 After the `call` function ended, the `back` function will be invoked,
 the return value of `call` function will be passed as a parameter in the
 `back`.

 Example:

```javascript
async (function () {
    sleep (5);
    return 42;
}, function (result) {
    console.log ("The answer to life, the universe and everything is",
                 result);
});
```

### sleep (seconds)

 Same with `man 3 sleep`.

 `sleep()` makes the calling process sleep until `seconds` seconds have
 elapsed. `sleep()` will ignore signals.

### fork ()

 Same with `man 2 fork`.

 `fork()` creates a new process by duplicating the calling process. The
 new process, referred to as the `child`, is an exact duplicate of the
 calling process, referred to as the parent.

 The PID of the child process is returned in the parent, and
 0 is returned in the child.

## BUG

 The project is not done yet, it has bugs and no unit tests, and it will
 crash when `call` throws exceptions...

 And `node-curl` uses processes instead of threads to invoke the `call`,
 it will be a bit resource-consuming, this is because v8 engine does not
 support threads good enough. When v8 finally supports threads, I will
 make an equivalent threading one.
