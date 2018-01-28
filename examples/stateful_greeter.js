'use strict';

var List = require("bs-platform/lib/js/list.js");
var Nact = require("../src/Nact.js");

var system = Nact.start(/* None */0, /* None */0, /* () */0);

var statefulGreeter = Nact.spawn(/* Some */["stateful-greeter"], /* None */0, /* None */0, system, (function (state, param, ctx) {
        var name = param[/* name */0];
        var hasPreviouslyGreetedMe = List.exists((function (v) {
                return +(v === name);
              }), state);
        if (hasPreviouslyGreetedMe) {
          console.log("Hello again " + name);
          return Promise.resolve(state);
        } else {
          console.log("Good to meet you, " + (name + (". I am the " + (ctx[/* name */4] + " service!"))));
          return Promise.resolve(/* :: */[
                      name,
                      state
                    ]);
        }
      }), /* [] */0);

Nact.Operators[/* <-< */0](statefulGreeter, /* record */[/* name */"Erlich"]);

Nact.Operators[/* <-< */0](statefulGreeter, /* record */[/* name */"Erlich"]);

Nact.Operators[/* <-< */0](statefulGreeter, /* record */[/* name */"Dinesh"]);

exports.system          = system;
exports.statefulGreeter = statefulGreeter;
/* system Not a pure module */
