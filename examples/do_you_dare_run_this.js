var fork = require("../fork").fork;

for (var i = 0; i < 20; i++) {
    fork ();

    console.log (i);
};
