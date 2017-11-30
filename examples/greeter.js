'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

var greeter = Nact.spawnStateless(/* Some */["greeter"], system, (function (param, _) {
        console.log("Hello " + param[/* name */0]);
        return /* () */0;
      }));

Nact.dispatch(/* None */0, greeter, /* record */[/* name */"Erlich Bachman"]);

exports.system  = system;
exports.greeter = greeter;
/* system Not a pure module */
