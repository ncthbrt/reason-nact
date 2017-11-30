'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

var ping = Nact.spawnStateless(/* Some */["ping"], system, (function (msg, ctx) {
        console.log(msg);
        var match = ctx[/* sender */0];
        if (match) {
          return Nact.dispatch(/* Some */[ctx[/* self */3]], match[0], ctx[/* name */5]);
        } else {
          return /* () */0;
        }
      }));

var pong = Nact.spawnStateless(/* Some */["pong"], system, (function (msg, ctx) {
        console.log(msg);
        var match = ctx[/* sender */0];
        if (match) {
          return Nact.dispatch(/* Some */[ctx[/* self */3]], match[0], ctx[/* name */5]);
        } else {
          return /* () */0;
        }
      }));

Nact.dispatch(/* Some */[pong], ping, "hello");

setTimeout((function () {
        return Nact.stop(system);
      }), 100);

exports.system = system;
exports.ping   = ping;
exports.pong   = pong;
/* system Not a pure module */
