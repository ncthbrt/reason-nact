'use strict';

var List = require("bs-platform/lib/js/list.js");
var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

var statefulGreeter = Nact.spawn(/* Some */["stateful-greeter"], system, (function (state, param, ctx) {
        var name = param[/* name */0];
        var hasPreviouslyGreetedMe = List.exists((function (v) {
                return +(v === name);
              }), state);
        if (hasPreviouslyGreetedMe) {
          console.log("Hello again " + name);
          return state;
        } else {
          console.log("Good to meet you, " + name + ". I am the " + ctx[/* name */5] + " service!");
          return /* :: */[
                  name,
                  state
                ];
        }
      }), /* [] */0);

Nact.dispatch(/* None */0, statefulGreeter, /* record */[/* name */"Erlich"]);

Nact.dispatch(/* None */0, statefulGreeter, /* record */[/* name */"Erlich"]);

Nact.dispatch(/* None */0, statefulGreeter, /* record */[/* name */"Dinesh"]);

exports.system          = system;
exports.statefulGreeter = statefulGreeter;
/* system Not a pure module */
