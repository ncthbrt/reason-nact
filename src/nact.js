'use strict';

var Nact              = require("nact");
var Curry             = require("bs-platform/lib/js/curry.js");
var Caml_int32        = require("bs-platform/lib/js/caml_int32.js");
var Nact_jsMap        = require("./Nact_jsMap.js");
var Nact_stringSet    = require("./Nact_stringSet.js");
var Caml_exceptions   = require("bs-platform/lib/js/caml_exceptions.js");
var Js_null_undefined = require("bs-platform/lib/js/js_null_undefined.js");

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
  var match = untypedCtx.recovering;
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* name */untypedCtx.name,
          /* persist */Curry.__1(partial_arg),
          /* children */Nact_stringSet.fromJsArray(Nact_jsMap.keys(untypedCtx.children)),
          /* recovering */(match == null) ? /* false */0 : match
        ];
}

function spawn(name, shutdownAfter, param, func, initialState) {
  var parent = param[0];
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter)
  };
  var f = function (possibleState, msg, ctx) {
    var state = (possibleState == null) ? initialState : possibleState;
    return Curry._3(func, state, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawn(parent, f, name[0], options) : Nact.spawn(parent, f, undefined, options);
  return /* ActorRef */[untypedRef];
}

function spawnStateless(name, shutdownAfter, param, func) {
  var parent = param[0];
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter)
  };
  var f = function (msg, ctx) {
    return Curry._2(func, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawnStateless(parent, f, name[0], options) : Nact.spawnStateless(parent, f, undefined, options);
  return /* ActorRef */[untypedRef];
}

function spawnPersistent(key, name, shutdownAfter, snapshotEvery, param, func, initialState) {
  var parent = param[0];
  var options = {
    shutdownAfter: Js_null_undefined.from_opt(shutdownAfter),
    snapshotEvery: Js_null_undefined.from_opt(snapshotEvery)
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
  var untypedRef = persistenceEngine ? Nact.start(Nact.configurePersistence(persistenceEngine[0])) : Nact.start();
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

var StringSet = 0;

var milliseconds = 1;

var millisecond = 1;

var second = seconds;

var minute = minutes;

var messages = 1;

var message = 1;

exports.StringSet       = StringSet;
exports.spawn           = spawn;
exports.spawnStateless  = spawnStateless;
exports.spawnPersistent = spawnPersistent;
exports.stop            = stop;
exports.start           = start;
exports.dispatch        = dispatch;
exports.QueryTimeout    = QueryTimeout;
exports.query           = query;
exports.milliseconds    = milliseconds;
exports.millisecond     = millisecond;
exports.seconds         = seconds;
exports.second          = second;
exports.minutes         = minutes;
exports.minute          = minute;
exports.hours           = hours;
exports.messages        = messages;
exports.message         = message;
/* nact Not a pure module */
