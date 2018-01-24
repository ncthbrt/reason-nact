'use strict';

var Nact                    = require("nact");
var Block                   = require("bs-platform/lib/js/block.js");
var Curry                   = require("bs-platform/lib/js/curry.js");
var Js_exn                  = require("bs-platform/lib/js/js_exn.js");
var $$String                = require("bs-platform/lib/js/string.js");
var Caml_int32              = require("bs-platform/lib/js/caml_int32.js");
var Nact_jsMap              = require("./Nact_jsMap.js");
var Js_primitive            = require("bs-platform/lib/js/js_primitive.js");
var Nact_stringSet          = require("./Nact_stringSet.js");
var Caml_exceptions         = require("bs-platform/lib/js/caml_exceptions.js");
var Js_null_undefined       = require("bs-platform/lib/js/js_null_undefined.js");
var References              = require("nact/lib/references");
var Caml_builtin_exceptions = require("bs-platform/lib/js/caml_builtin_exceptions.js");

function defaultTo($$default, opt) {
  if (opt) {
    return opt[0];
  } else {
    return $$default;
  }
}

function fromReference(param) {
  return /* ActorPath */[param[0].path];
}

function systemName(param) {
  return param[0].system;
}

function toString(param) {
  var path = param[0];
  return "system:" + (path.system + ("//" + $$String.concat("/", path.parts)));
}

var ActorPath = /* module */[
  /* fromReference */fromReference,
  /* systemName */systemName,
  /* toString */toString
];

function logLevelFromJs(param) {
  if (param <= 6 && 0 <= param) {
    return /* Some */[param - 0 | 0];
  } else {
    return /* None */0;
  }
}

function fromJsLog(msg) {
  var path = /* ActorPath */[msg.actor.path];
  var createdAt = msg.createdAt;
  var match = msg.type;
  switch (match) {
    case "event" : 
        return /* Event */Block.__(3, [
                  defaultTo("", Js_primitive.null_undefined_to_opt(msg.name)),
                  defaultTo(null, Js_primitive.null_undefined_to_opt(msg.properties)),
                  createdAt,
                  path
                ]);
    case "exception" : 
        return /* Error */Block.__(1, [
                  defaultTo([
                        Caml_builtin_exceptions.failure,
                        "Error is undefined"
                      ], Js_primitive.null_undefined_to_opt(msg.exception)),
                  createdAt,
                  path
                ]);
    case "metric" : 
        return /* Metric */Block.__(2, [
                  defaultTo("", Js_primitive.null_undefined_to_opt(msg.name)),
                  defaultTo(null, Js_primitive.null_undefined_to_opt(msg.values)),
                  createdAt,
                  path
                ]);
    case "trace" : 
        return /* Message */Block.__(0, [
                  defaultTo(/* Off */0, logLevelFromJs(msg.level)),
                  defaultTo("", Js_primitive.null_undefined_to_opt(msg.message)),
                  createdAt,
                  path
                ]);
    default:
      return /* Unknown */Block.__(4, [
                msg,
                createdAt,
                path
              ]);
  }
}

function trace(message, loggingEngine) {
  loggingEngine.trace(message);
  return /* () */0;
}

function debug(message, loggingEngine) {
  loggingEngine.debug(message);
  return /* () */0;
}

function info(message, loggingEngine) {
  loggingEngine.info(message);
  return /* () */0;
}

function warn(message, loggingEngine) {
  loggingEngine.warn(message);
  return /* () */0;
}

function error(message, loggingEngine) {
  loggingEngine.error(message);
  return /* () */0;
}

function critical(message, loggingEngine) {
  loggingEngine.critical(message);
  return /* () */0;
}

function $$event(name, properties, loggingEngine) {
  loggingEngine.event(name, properties);
  return /* () */0;
}

function metric(name, values, loggingEngine) {
  loggingEngine.metric(name, values);
  return /* () */0;
}

function exception_(err, loggingEngine) {
  loggingEngine.exception(err);
  return /* () */0;
}

function mapCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* children */Nact_stringSet.fromJsArray(Nact_jsMap.keys(untypedCtx.children)),
          /* name */untypedCtx.name,
          /* logger */untypedCtx.log
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
          /* recovering */defaultTo(/* false */0, Js_primitive.null_undefined_to_opt(untypedCtx.recovering)),
          /* logger */untypedCtx.log
        ];
}

function mapSupervisionCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* child */untypedCtx.child.name,
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* name */untypedCtx.name,
          /* children */Nact_stringSet.fromJsArray(Nact_jsMap.keys(untypedCtx.children))
        ];
}

function mapSupervisionFunction(optionalF) {
  if (optionalF) {
    var f = optionalF[0];
    return (function (_, err, ctx) {
        return Curry._2(f, err, mapSupervisionCtx(ctx)).then((function (decision) {
                      var tmp;
                      switch (decision) {
                        case 0 : 
                            tmp = ctx.stop;
                            break;
                        case 1 : 
                            tmp = ctx.stopAll;
                            break;
                        case 2 : 
                            tmp = ctx.reset;
                            break;
                        case 3 : 
                            tmp = ctx.resetAll;
                            break;
                        case 4 : 
                            tmp = ctx.escalate;
                            break;
                        case 5 : 
                            tmp = ctx.resume;
                            break;
                        
                      }
                      return Promise.resolve(tmp);
                    }));
      });
  } else {
    return undefined;
  }
}

function useStatefulSupervisionPolicy(f, initialState) {
  var state = [initialState];
  return (function (err, ctx) {
      var match = Curry._3(f, err, state[0], ctx);
      state[0] = match[0];
      return match[1];
    });
}

function spawn(name, shutdownAfter, whenChildCrashes, decoder, param, func, initialState) {
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter),
    whenChildCrashes: mapSupervisionFunction(whenChildCrashes)
  };
  var decoder$1 = defaultTo((function (prim) {
          return prim;
        }), decoder);
  var f = function (possibleState, msg, ctx) {
    var state = defaultTo(initialState, (possibleState == null) ? /* None */0 : [possibleState]);
    try {
      return Curry._3(func, state, Curry._1(decoder$1, msg), mapCtx(ctx));
    }
    catch (raw_err){
      return Promise.reject(Js_exn.internalToOCamlException(raw_err));
    }
  };
  var untypedRef = Nact.spawn(param[0], f, Js_null_undefined.from_opt(name), options);
  return /* ActorRef */[untypedRef];
}

function spawnStateless(name, shutdownAfter, whenChildCrashes, decoder, param, func) {
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter),
    whenChildCrashes: mapSupervisionFunction(whenChildCrashes)
  };
  var decoder$1 = defaultTo((function (prim) {
          return prim;
        }), decoder);
  var f = function (msg, ctx) {
    return Curry._2(func, Curry._1(decoder$1, msg), mapCtx(ctx));
  };
  var untypedRef = Nact.spawnStateless(param[0], f, Js_null_undefined.from_opt(name), options);
  return /* ActorRef */[untypedRef];
}

function spawnPersistent(key, name, shutdownAfter, snapshotEvery, whenChildCrashes, decoder, stateDecoder, stateEncoder, param, func, initialState) {
  var decoder$1 = defaultTo((function (prim) {
          return prim;
        }), decoder);
  var stateDecoder$1 = defaultTo((function (prim) {
          return prim;
        }), stateDecoder);
  var stateEncoder$1 = defaultTo((function (prim) {
          return prim;
        }), stateEncoder);
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter),
    snapshotEvery: Js_null_undefined.from_opt(snapshotEvery),
    whenChildCrashes: mapSupervisionFunction(whenChildCrashes)
  };
  var f = function (state, msg, ctx) {
    var state$1 = (state == null) ? initialState : Curry._1(stateDecoder$1, state);
    var tmp;
    try {
      tmp = Curry._3(func, state$1, Curry._1(decoder$1, msg), mapPersistentCtx(ctx));
    }
    catch (raw_err){
      tmp = Promise.reject(Js_exn.internalToOCamlException(raw_err));
    }
    return tmp.then((function (result) {
                  return Promise.resolve(Curry._1(stateEncoder$1, result));
                }));
  };
  var untypedRef = Nact.spawnPersistent(param[0], f, key, Js_null_undefined.from_opt(name), options);
  return /* ActorRef */[untypedRef];
}

function stop(param) {
  Nact.stop(param[0]);
  return /* () */0;
}

function dispatch(param, msg) {
  Nact.dispatch(param[0], msg);
  return /* () */0;
}

function nobody() {
  return /* ActorRef */[new References.Nobody()];
}

function spawnAdapter(parent, mapping) {
  return spawnStateless(/* None */0, /* None */0, /* None */0, /* None */0, parent, (function (msg, _) {
                return Promise.resolve(dispatch(parent, Curry._1(mapping, msg)));
              }));
}

function mapLoggingActor(loggingActorFunction, system) {
  var loggerActor = Curry._1(loggingActorFunction, /* ActorRef */[system]);
  return spawnAdapter(loggerActor, fromJsLog)[0];
}

function start(persistenceEngine, logger, _) {
  var untypedRef;
  if (persistenceEngine) {
    var persistence = persistenceEngine[0];
    if (logger) {
      var logger$1 = logger[0];
      untypedRef = Nact.start(Nact.configureLogging((function (param) {
                  return mapLoggingActor(logger$1, param);
                })), Nact.configurePersistence(persistence));
    } else {
      untypedRef = Nact.start(Nact.configurePersistence(persistence));
    }
  } else if (logger) {
    var logger$2 = logger[0];
    untypedRef = Nact.start(Nact.configureLogging((function (param) {
                return mapLoggingActor(logger$2, param);
              })));
  } else {
    untypedRef = Nact.start();
  }
  return /* ActorRef */[untypedRef];
}

var QueryTimeout = Caml_exceptions.create("Nact.QueryTimeout");

function query(timeout, param, msgF) {
  var f = function (tempReference) {
    return Curry._1(msgF, /* ActorRef */[tempReference]);
  };
  return Nact.query(param[0], f, timeout).catch((function () {
                return Promise.reject([
                            QueryTimeout,
                            timeout
                          ]);
              }));
}

var seconds = 1000;

var minutes = Caml_int32.imul(60, seconds);

var hours = Caml_int32.imul(60, minutes);

var $less$neg$less = dispatch;

function $great$neg$great(msg, actorRef) {
  return dispatch(actorRef, msg);
}

function $less$unknown(actor, param) {
  return query(param[1], actor, param[0]);
}

var Operators = /* module */[
  /* <-< */$less$neg$less,
  /* >-> */$great$neg$great,
  /* <? */$less$unknown
];

var StringSet = 0;

var Log = [
  trace,
  debug,
  info,
  warn,
  error,
  critical,
  $$event,
  exception_,
  metric
];

var milliseconds = 1;

var millisecond = 1;

var second = seconds;

var minute = minutes;

var messages = 1;

var message = 1;

exports.StringSet                    = StringSet;
exports.ActorPath                    = ActorPath;
exports.Log                          = Log;
exports.useStatefulSupervisionPolicy = useStatefulSupervisionPolicy;
exports.spawn                        = spawn;
exports.spawnStateless               = spawnStateless;
exports.spawnPersistent              = spawnPersistent;
exports.spawnAdapter                 = spawnAdapter;
exports.start                        = start;
exports.stop                         = stop;
exports.dispatch                     = dispatch;
exports.nobody                       = nobody;
exports.QueryTimeout                 = QueryTimeout;
exports.query                        = query;
exports.milliseconds                 = milliseconds;
exports.millisecond                  = millisecond;
exports.seconds                      = seconds;
exports.second                       = second;
exports.minutes                      = minutes;
exports.minute                       = minute;
exports.hours                        = hours;
exports.messages                     = messages;
exports.message                      = message;
exports.Operators                    = Operators;
/* nact Not a pure module */
