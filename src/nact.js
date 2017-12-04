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

function spawn(name, param, func, initialState) {
  var parent = param[0];
  var f = function (possibleState, msg, ctx) {
    var state = (possibleState == null) ? initialState : possibleState;
    return Curry._3(func, state, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawn(parent, f, name[0]) : Nact.spawn(parent, f, undefined);
  return /* ActorRef */[untypedRef];
}

function spawnStateless(name, param, func) {
  var parent = param[0];
  var f = function (msg, ctx) {
    return Curry._2(func, msg, mapCtx(ctx));
  };
  var untypedRef = name ? Nact.spawnStateless(parent, f, name[0]) : Nact.spawnStateless(parent, f, undefined);
  return /* ActorRef */[untypedRef];
}

function spawnPersistent(key, name, param, func, initialState) {
  var parent = param[0];
  var f = function (possibleState, msg, ctx) {
    var state = (possibleState == null) ? initialState : possibleState;
    return Curry._3(func, state, msg, mapPersistentCtx(ctx));
  };
  var untypedRef = name ? Nact.spawnPersistent(parent, f, key, name[0]) : Nact.spawnPersistent(parent, f, key, undefined);
  return /* ActorRef */[untypedRef];
}

function stop(param) {
  Nact.stop(param[0]);
  return /* () */0;
}

function start() {
  var untypedRef = Nact.start();
  return /* ActorRef */[untypedRef];
}

function dispatch(param, msg) {
  Nact.dispatch(param[0], msg);
  return /* () */0;
}

var QueryTimeout = Caml_exceptions.create("Nact.QueryTimeout");

var ActorNotAvailable = Caml_exceptions.create("Nact.ActorNotAvailable");

function query(timeout, param, msgF) {
  var recipient = param[0];
  return Nact.query(recipient, Curry._1(msgF, /* ActorRef */[recipient]), timeout).catch((function () {
                return Promise.reject([
                            QueryTimeout,
                            timeout
                          ]);
              }));
}

var StringSet = 0;

exports.StringSet         = StringSet;
exports.spawn             = spawn;
exports.spawnStateless    = spawnStateless;
exports.spawnPersistent   = spawnPersistent;
exports.stop              = stop;
exports.start             = start;
exports.dispatch          = dispatch;
exports.ActorNotAvailable = ActorNotAvailable;
exports.QueryTimeout      = QueryTimeout;
exports.query             = query;
/* nact Not a pure module */
