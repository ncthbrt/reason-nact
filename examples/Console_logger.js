'use strict';

var Nact         = require("../src/Nact.js");
var $$String     = require("bs-platform/lib/js/string.js");
var Js_primitive = require("bs-platform/lib/js/js_primitive.js");

function defaultTo($$default, param) {
  if (param) {
    return param[0];
  } else {
    return $$default;
  }
}

function getLogText(param) {
  switch (param.tag | 0) {
    case 0 : 
        var pathStr = Nact.ActorPath[/* toString */2](param[3]);
        var dateStr = param[2].toUTCString();
        var levelStr = $$String.uppercase(Nact.Log[/* logLevelToString */0](param[0]));
        return /* tuple */[
                levelStr,
                pathStr,
                dateStr,
                param[1]
              ];
    case 1 : 
        var pathStr$1 = Nact.ActorPath[/* toString */2](param[2]);
        var dateStr$1 = param[1].toUTCString();
        var json = defaultTo("", Js_primitive.undefined_to_opt(JSON.stringify(param[0])));
        return /* tuple */[
                "EXCEPTION",
                pathStr$1,
                dateStr$1,
                json
              ];
    case 2 : 
        var pathStr$2 = Nact.ActorPath[/* toString */2](param[3]);
        var dateStr$2 = param[2].toUTCString();
        var json$1 = JSON.stringify(param[1]);
        return /* tuple */[
                "METRIC",
                pathStr$2,
                dateStr$2,
                "{ \"" + (String(param[0]) + ("\": " + (String(json$1) + " }")))
              ];
    case 3 : 
        var pathStr$3 = Nact.ActorPath[/* toString */2](param[3]);
        var dateStr$3 = param[2].toUTCString();
        var json$2 = JSON.stringify(param[1]);
        return /* tuple */[
                "EVENT",
                pathStr$3,
                dateStr$3,
                "{ \"" + (String(param[0]) + ("\": " + (String(json$2) + " }")))
              ];
    case 4 : 
        var pathStr$4 = Nact.ActorPath[/* toString */2](param[2]);
        var dateStr$4 = param[1].toUTCString();
        var text = JSON.stringify(param[0]);
        return /* tuple */[
                "???",
                pathStr$4,
                dateStr$4,
                text
              ];
    
  }
}

function consoleLogger(system) {
  return Nact.spawnStateless(/* Some */["console-logger"], /* None */0, /* None */0, system, (function (msg, _) {
                var match = getLogText(msg);
                console.log("[" + (String(match[0]) + (", " + (String(match[1]) + (", " + (String(match[2]) + ("]: " + (String(match[3]) + ""))))))));
                return Promise.resolve(/* () */0);
              }));
}

var system = Nact.start(/* None */0, /* Some */[consoleLogger], /* () */0);

var stringClassifierActor = Nact.spawnStateless(/* None */0, /* None */0, /* None */0, system, (function (msg, ctx) {
        return Promise.resolve("mutation".indexOf($$String.lowercase(msg)) >= 0 ? Nact.Log[/* event */7]("receivedEvilMessage", msg, ctx[/* logger */5]) : Nact.Log[/* info */3]("Received message: " + msg, ctx[/* logger */5]));
      }));

exports.defaultTo             = defaultTo;
exports.getLogText            = getLogText;
exports.consoleLogger         = consoleLogger;
exports.system                = system;
exports.stringClassifierActor = stringClassifierActor;
/* system Not a pure module */
