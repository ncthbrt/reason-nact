'use strict';

var Nact            = require("nact");
var Curry           = require("bs-platform/lib/js/curry.js");
var Nact_jsMap      = require("./Nact_jsMap.js");
var Nact_stringSet  = require("./Nact_stringSet.js");
var Caml_exceptions = require("bs-platform/lib/js/caml_exceptions.js");

function mapCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* children */Nact_stringSet.fromJsArray(Nact_jsMap.keys(untypedCtx.children)),
          /* name */untypedCtx.name
        ];
}

function mapPersistentCtx(untypedCtx) {
  var partial_arg = untypedCtx.persist;
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* name */untypedCtx.name,
          /* persist */Curry.__1(partial_arg),
          /* children */Nact_stringSet.fromJsArray(Nact_jsMap.keys(untypedCtx.children)),
          /* recovering */untypedCtx.recovering
        ];
}

function defaultOrValue(opt, defaultValue) {
  if (opt) {
    return opt[0];
  } else {
    return defaultValue;
  }
}

function after(hours, minutes, seconds, milliseconds, _) {
  return /* record */[/* duration */((defaultOrValue(hours, 0) + defaultOrValue(minutes, 0) | 0) + defaultOrValue(seconds, 0) | 0) + defaultOrValue(milliseconds, 0) | 0];
}

function every(messages, hours, minutes, seconds, milliseconds, _) {
  return /* record */[
          /* duration */((defaultOrValue(hours, 0) + defaultOrValue(minutes, 0) | 0) + defaultOrValue(seconds, 0) | 0) + defaultOrValue(milliseconds, 0) | 0,
          /* messages */defaultOrValue(messages, 0)
        ];
}

function spawn(name, timeout, param, func, initialState) {
  var parent = param[0];
  var duration = timeout ? ({
        duration: timeout[0][/* duration */0]
      }) : undefined;
  var options = {
    timeout: duration
  };
  var f = function (possibleState, msg, ctx) {
    var state = (possibleState == null) ? initialState : possibleState;
    return Curry._3(func, state, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawn(parent, f, name[0], options) : Nact.spawn(parent, f, undefined, options);
  return /* ActorRef */[untypedRef];
}

function spawnStateless(name, timeout, param, func) {
  var parent = param[0];
  var timeout$1 = timeout ? ({
        duration: timeout[0][/* duration */0]
      }) : undefined;
  var options = {
    timeout: timeout$1
  };
  var f = function (msg, ctx) {
    return Curry._2(func, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawnStateless(parent, f, name[0], options) : Nact.spawnStateless(parent, f, undefined, options);
  return /* ActorRef */[untypedRef];
}

function spawnPersistent(key, name, timeout, snapshot, param, func, initialState) {
  var parent = param[0];
  var timeout$1 = timeout ? ({
        duration: timeout[0][/* duration */0]
      }) : undefined;
  var snapshot$1;
  if (snapshot) {
    var match = snapshot[0];
    var duration = match[/* duration */0];
    var exit = 0;
    if (duration !== 0 || match[/* messages */1] !== 0) {
      exit = 1;
    } else {
      snapshot$1 = undefined;
    }
    if (exit === 1) {
      var messages = match[/* messages */1];
      snapshot$1 = messages !== 0 ? ({
            duration: duration,
            messages: messages
          }) : ({
            duration: duration,
            messages: undefined
          });
    }
    
  } else {
    snapshot$1 = undefined;
  }
  var options = {
    timeout: timeout$1,
    snapshot: snapshot$1
  };
  var f = function (possibleState, msg, ctx) {
    var state = (possibleState == null) ? initialState : possibleState;
    return Curry._3(func, state, msg, mapPersistentCtx(ctx));
  };
  var untypedRef = name ? Nact.spawnPersistent(parent, f, key, name[0], options) : Nact.spawnPersistent(parent, f, key, undefined, options);
  return /* ActorRef */[untypedRef];
}

function stop(param) {
  Nact.stop(param[0]);
  return /* () */0;
}

function start(persistenceEngine, _) {
  var untypedRef;
  if (persistenceEngine) {
    var engine = persistenceEngine[0];
    untypedRef = Nact.start((function (param) {
            Nact.configurePersistence(engine, param);
            return /* () */0;
          }));
  } else {
    untypedRef = Nact.start();
  }
  return /* ActorRef */[untypedRef];
}

function dispatch(param, msg) {
  Nact.dispatch(param[0], msg);
  return /* () */0;
}

var QueryTimeout = Caml_exceptions.create("Nact.QueryTimeout");

function query(timeout, param, msgF) {
  var f = function (tempReference) {
    return Curry._1(msgF, /* ActorRef */[tempReference]);
  };
  return Nact.query(param[0], f, timeout[/* duration */0]).catch((function () {
                return Promise.reject([
                            QueryTimeout,
                            timeout
                          ]);
              }));
}

var StringSet = 0;

exports.StringSet       = StringSet;
exports.after           = after;
exports.every           = every;
exports.spawn           = spawn;
exports.spawnStateless  = spawnStateless;
exports.spawnPersistent = spawnPersistent;
exports.stop            = stop;
exports.start           = start;
exports.dispatch        = dispatch;
exports.QueryTimeout    = QueryTimeout;
exports.query           = query;
/* nact Not a pure module */
