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
