'use strict';

var Nact = require("../src/nact.js");

var system = Nact.start(/* () */0);

function hasPreviouslyBeenGreeted(name, _lst) {
  while(true) {
    var lst = _lst;
    if (lst) {
      var match = +(lst[0] === name);
      if (match !== 0) {
        return /* true */1;
      } else {
        _lst = lst[1];
        continue ;
        
      }
    } else {
      return /* false */0;
    }
  };
}

var statefulGreeter = Nact.spawn(/* Some */["stateful-greeter"], system, (function (state, param, ctx) {
        var name = param[/* name */0];
        var hasPreviouslyGreetedMe = hasPreviouslyBeenGreeted(name, state);
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

exports.system                   = system;
exports.hasPreviouslyBeenGreeted = hasPreviouslyBeenGreeted;
exports.statefulGreeter          = statefulGreeter;
/* system Not a pure module */
