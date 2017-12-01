'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

var ping = Nact.spawnStateless(/* Some */["ping"], system, (function (msg, ctx) {
        console.log(msg);
        return Nact.optionallyDispatch(/* Some */[ctx[/* self */3]], ctx[/* sender */0], ctx[/* name */5]);
      }));

var pong = Nact.spawnStateless(/* Some */["pong"], system, (function (msg, ctx) {
        console.log(msg);
        return Nact.optionallyDispatch(/* Some */[ctx[/* self */3]], ctx[/* sender */0], ctx[/* name */5]);
      }));

Nact.dispatch(/* Some */[pong], ping, "hello");

setTimeout((function () {
        return Nact.stop(system);
      }), 100);

exports.system = system;
exports.ping   = ping;
exports.pong   = pong;
/* system Not a pure module */
