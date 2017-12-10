'use strict';

var Nact  = require("../src/nact.js");
var Curry = require("bs-platform/lib/js/curry.js");

var $less$neg$less = Nact.Operators[0];

var system = Nact.start(/* None */0, /* () */0);

var ping = Nact.spawnStateless(/* Some */["ping"], /* None */0, system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Curry._2($less$neg$less, param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

var pong = Nact.spawnStateless(/* Some */["pong"], /* None */0, system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Curry._2($less$neg$less, param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

Curry._2($less$neg$less, ping, /* Msg */[
      pong,
      "hello"
    ]);

setTimeout((function () {
        return Nact.stop(system);
      }), 100);

var $great$neg$great = Nact.Operators[1];

exports.$less$neg$less   = $less$neg$less;
exports.$great$neg$great = $great$neg$great;
exports.system           = system;
exports.ping             = ping;
exports.pong             = pong;
/* system Not a pure module */
