var fork = require("../fork").fork;

for (var i = 0; i < 5; i++) {
    fork ();

    console.log (i);
};
