var curl = require("./node_curl");
var fork = require("./fork").fork;
var sleep = require("./fork").sleep;
var async = require("./fork").async;

async (function () {
    var res = curl.get ("http://sina.com").end();
    console.log (res.headers);
});

async (function () {
    var res = curl.get ("http://weibo.com").end();
    console.log (res.headers);
});

require("http").createServer(function (req, res) {
}).listen(9001);

//var pid = fork ();

//if (pid == 0) {
//    sleep (5);
//    console.log (pid);
//    var res = curl.get ("http://sina.com").end();
//    console.log (res.headers);
//} else {
//    console.log (pid);
//    var res = curl.get ("http://weibo.com").end();
//    console.log (res.headers);
//}
