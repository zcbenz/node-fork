var curl   = require("./node_curl");

var fork   = require("./fork").fork;
var sleep  = require("./fork").sleep;
var async  = require("./fork").async;
var future = require("./fork").future;

var all = future (function () {
    console.log ("sina");
    curl.get ("http://sina.com").end();
    console.log ("baidu");
    curl.get ("http://baidu.com").end();
//    console.log ("google");
//    curl.get ("http://google.com").end();
    console.log ("weibo");
    curl.get ("http://weibo.com").end();
    console.log ("163");
    curl.get ("http://163.com").end();
    console.log ("sohu");
    curl.get ("http://sohu.com").end();
    console.log ("cnbeta");
    var res = curl.get ("http://cnbeta.com").end();

    return res.headers;
});

var sohu = future (function () {
    console.log ("sohu");
    return curl.get ("http://sohu.com").end ().headers;
});

console.log (all.get ());
console.log (sohu.get ());

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
