'use strict';

var Nact$ReasonNact = require("./nact.js");

var system = Nact$ReasonNact.start(/* () */0);

var ping = Nact$ReasonNact.spawnStateless(/* Some */["ping"], system, (function (msg, ctx) {
        console.log(msg);
        var match = ctx[/* sender */0];
        if (match) {
          return Nact$ReasonNact.dispatch(/* Some */[ctx[/* self */3]], match[0], ctx[/* name */4]);
        } else {
          console.log("Saying goodbye now");
          return /* () */0;
        }
      }));

var pong = Nact$ReasonNact.spawnStateless(/* Some */["pong"], system, (function (msg, ctx) {
        console.log(msg);
        var match = ctx[/* sender */0];
        if (match) {
          return Nact$ReasonNact.dispatch(/* Some */[ctx[/* self */3]], match[0], ctx[/* name */4]);
        } else {
          console.log("Saying goodbye now");
          return /* () */0;
        }
      }));

Nact$ReasonNact.dispatch(/* Some */[pong], ping, "hello");

exports.system = system;
exports.ping   = ping;
exports.pong   = pong;
/* system Not a pure module */
