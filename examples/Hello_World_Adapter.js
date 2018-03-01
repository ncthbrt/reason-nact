'use strict';

var Nact = require("../src/Nact.js");
var Pervasives = require("bs-platform/lib/js/pervasives.js");

var system = Nact.start(/* None */0, /* None */0, /* () */0);

var world = Nact.spawnStateless(/* Some */["world"], /* None */0, /* None */0, system, (function (param, ctx) {
        console.log("world!!");
        return Promise.resolve(Nact.Operators[/* <-< */0](param[0], /* World */[ctx[/* self */2]]));
      }));

function createAdapterIfNotExists(parent, adapterOpt) {
  if (adapterOpt) {
    return adapterOpt[0];
  } else {
    return Nact.spawnAdapter(parent, (function (param) {
                  return /* Hello */[param[0]];
                }));
  }
}

var hello = Nact.spawn(/* Some */["hello"], /* None */0, /* None */0, system, (function (adapterOpt, param, ctx) {
        var adapter = createAdapterIfNotExists(ctx[/* self */2], adapterOpt);
        Pervasives.print_string("Hello ");
        Nact.Operators[/* <-< */0](param[0], /* World */[adapter]);
        return Promise.resolve(/* Some */[adapter]);
      }), /* None */0);

Nact.Operators[/* <-< */0](hello, /* Hello */[world]);

exports.system = system;
exports.world = world;
exports.createAdapterIfNotExists = createAdapterIfNotExists;
exports.hello = hello;
/* system Not a pure module */
