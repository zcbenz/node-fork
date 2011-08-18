var sleep = require("../fork").sleep;
var async = require("../fork").async;

for (var i = 0; i < 10; ++i) {
    async (function () {
        sleep (i);
        console.warn ("Pig " + i + " awakes!");
    });
}

// Just to block the main process
require("http").createServer(function (req, res) {
}).listen(9001);
