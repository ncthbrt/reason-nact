'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

var ping = Nact.spawnStateless(/* Some */["ping"], system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Nact.dispatch(param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

var pong = Nact.spawnStateless(/* Some */["pong"], system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Nact.dispatch(param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

Nact.dispatch(ping, /* Msg */[
      pong,
      "hello"
    ]);

setTimeout((function () {
        return Nact.stop(system);
      }), 100);

exports.system = system;
exports.ping   = ping;
exports.pong   = pong;
/* system Not a pure module */
