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
