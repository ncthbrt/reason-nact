'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* None */0, /* () */0);

var greeter = Nact.spawnStateless(/* Some */["greeter"], system, (function (param, _) {
        return Promise.resolve((console.log("Hello " + param[/* name */0]), /* () */0));
      }));

Nact.dispatch(greeter, /* record */[/* name */"Erlich Bachman"]);

exports.system  = system;
exports.greeter = greeter;
/* system Not a pure module */
