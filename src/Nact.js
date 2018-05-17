'use strict';

var Nact = require("nact");
var Curry = require("bs-platform/lib/js/curry.js");
var Js_exn = require("bs-platform/lib/js/js_exn.js");
var $$String = require("bs-platform/lib/js/string.js");
var Belt_List = require("bs-platform/lib/js/belt_List.js");
var Caml_int32 = require("bs-platform/lib/js/caml_int32.js");
var Nact_jsMap = require("./Nact_jsMap.js");
var Belt_Option = require("bs-platform/lib/js/belt_Option.js");
var Js_primitive = require("bs-platform/lib/js/js_primitive.js");
var Belt_SetString = require("bs-platform/lib/js/belt_SetString.js");
var Caml_exceptions = require("bs-platform/lib/js/caml_exceptions.js");
var Js_null_undefined = require("bs-platform/lib/js/js_null_undefined.js");
var References = require("nact/lib/references");

function fromUntypedRef(reference) {
  return /* ActorRef */[reference];
}

function toUntypedRef(param) {
  return param[0];
}

function dispatch(prim, prim$1) {
  Nact.dispatch(prim, prim$1);
  return /* () */0;
}

var Interop = /* module */[
  /* fromUntypedRef */fromUntypedRef,
  /* toUntypedRef */toUntypedRef,
  /* dispatch */dispatch
];

function fromReference(param) {
  return /* ActorPath */[param[0].path];
}

function systemName(param) {
  return param[0].system;
}

function toString(param) {
  var path = param[0];
  return "system:" + (path.system + ("//" + $$String.concat("/", Belt_List.fromArray(path.parts))));
}

function parts(param) {
  return Belt_List.fromArray(param[0].parts);
}

var ActorPath = /* module */[
  /* fromReference */fromReference,
  /* systemName */systemName,
  /* toString */toString,
  /* parts */parts
];


/* This code is to handle how bucklescript sometimes represents variants */

var WrappedVariant = '_wvariant';
var WrappedEvent = '_wevent';
function unsafeEncoder(obj) {
  var data = JSON.stringify(obj, function (key, value) {
    if (value && Array.isArray(value) && value.tag !== undefined) {
      var r = {};
      r.values = value.slice();
      r.tag = value.tag;
      r.type = WrappedVariant;
      return r;
    } else {
      return value;
    }
  });
  return { data: JSON.parse(data), type: WrappedEvent };
};

function unsafeDecoder(result) {
  if(result && typeof(result) === 'object' && result.type === WrappedEvent) {
    var serialized = result.serialized || JSON.stringify(result.data);
    return JSON.parse(serialized, (key, value) => {
      if (value && typeof (value) === 'object' && value.type === WrappedVariant) {
        var values = value.values;
        values.tag = value.tag;
        return values;
      } else {
        return value;
      }
    });
  } else {
    return result;
  }
};

;

function mapCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* children */Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children)),
          /* name */untypedCtx.name
        ];
}

function mapPersistentCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* name */untypedCtx.name,
          /* persist */untypedCtx.persist,
          /* children */Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children)),
          /* recovering */Belt_Option.getWithDefault(Js_primitive.null_undefined_to_opt(untypedCtx.recovering), false)
        ];
}

function mapSupervisionCtx(untypedCtx) {
  return /* record */[
          /* parent : ActorRef */[untypedCtx.parent],
          /* path : ActorPath */[untypedCtx.path],
          /* self : ActorRef */[untypedCtx.self],
          /* name */untypedCtx.name,
          /* children */Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children))
        ];
}

function mapSupervisionFunction(optionalF) {
  if (optionalF) {
    var f = optionalF[0];
    return (function (msg, err, ctx) {
        return Curry._3(f, msg, err, mapSupervisionCtx(ctx)).then((function (decision) {
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
  return (function (msg, err, ctx) {
      var match = Curry._4(f, msg, err, state[0], ctx);
      state[0] = match[0];
      return match[1];
    });
}

function spawn(name, shutdownAfter, onCrash, param, func, initialState) {
  var options = {
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    onCrash: mapSupervisionFunction(onCrash)
  };
  var f = function (possibleState, msg, ctx) {
    var state = Belt_Option.getWithDefault((possibleState == null) ? /* None */0 : [possibleState], initialState);
    try {
      return Curry._3(func, state, msg, mapCtx(ctx));
    }
    catch (raw_err){
      return Promise.reject(Js_exn.internalToOCamlException(raw_err));
    }
  };
  var untypedRef = Nact.spawn(param[0], f, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */[untypedRef];
}

function spawnStateless(name, shutdownAfter, param, func) {
  var options = {
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    onCrash: undefined
  };
  var f = function (msg, ctx) {
    try {
      return Curry._2(func, msg, mapCtx(ctx));
    }
    catch (raw_e){
      return Promise.reject(Js_exn.internalToOCamlException(raw_e));
    }
  };
  var untypedRef = Nact.spawnStateless(param[0], f, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */[untypedRef];
}

function spawnPersistent(key, name, shutdownAfter, snapshotEvery, onCrash, decoder, stateDecoder, encoder, stateEncoder, param, func, initialState) {
  var decoder$1 = Belt_Option.getWithDefault(decoder, (function (prim) {
          return unsafeDecoder(prim);
        }));
  var stateDecoder$1 = Belt_Option.getWithDefault(stateDecoder, (function (prim) {
          return unsafeDecoder(prim);
        }));
  var stateEncoder$1 = Belt_Option.getWithDefault(stateEncoder, (function (prim) {
          return unsafeEncoder(prim);
        }));
  var encoder$1 = Belt_Option.getWithDefault(encoder, (function (prim) {
          return unsafeEncoder(prim);
        }));
  var options = {
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    onCrash: mapSupervisionFunction(onCrash),
    snapshotEvery: Js_null_undefined.fromOption(snapshotEvery),
    encoder: encoder$1,
    decoder: decoder$1,
    snapshotEncoder: stateEncoder$1,
    snapshotDecoder: stateDecoder$1
  };
  var f = function (state, msg, ctx) {
    var state$1 = (state == null) ? initialState : state;
    try {
      return Curry._3(func, state$1, msg, mapPersistentCtx(ctx));
    }
    catch (raw_err){
      return Promise.reject(Js_exn.internalToOCamlException(raw_err));
    }
  };
  var untypedRef = Nact.spawnPersistent(param[0], f, key, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */[untypedRef];
}

function stop(param) {
  Nact.stop(param[0]);
  return /* () */0;
}

function dispatch$1(param, msg) {
  Nact.dispatch(param[0], msg);
  return /* () */0;
}

function nobody() {
  return /* ActorRef */[new References.Nobody()];
}

function spawnAdapter(name, parent, mapping) {
  var f = function (msg, _) {
    return Promise.resolve(dispatch$1(parent, Curry._1(mapping, msg)));
  };
  if (name) {
    return spawnStateless(/* Some */[name[0]], /* None */0, parent, f);
  } else {
    return spawnStateless(/* None */0, /* None */0, parent, f);
  }
}

function start(name, persistenceEngine, _) {
  var plugins = persistenceEngine ? /* :: */[
      Nact.configurePersistence(persistenceEngine[0]),
      /* [] */0
    ] : /* [] */0;
  var plugins$1 = name ? /* :: */[
      {
        name: name[0]
      },
      plugins
    ] : plugins;
  if (plugins$1) {
    var match = plugins$1[1];
    var a = plugins$1[0];
    if (match) {
      return /* ActorRef */[Nact.start(a, match[0])];
    } else {
      return /* ActorRef */[Nact.start(a)];
    }
  } else {
    return /* ActorRef */[Nact.start()];
  }
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

var $less$neg$less = dispatch$1;

function $great$neg$great(msg, actorRef) {
  return dispatch$1(actorRef, msg);
}

function $less$unknown(actor, param) {
  return query(param[1], actor, param[0]);
}

var Operators = /* module */[
  /* <-< */$less$neg$less,
  /* >-> */$great$neg$great,
  /* <? */$less$unknown
];

function unsafeDecoder$1(prim) {
  return unsafeDecoder(prim);
}

function unsafeEncoder$1(prim) {
  return unsafeEncoder(prim);
}

var milliseconds = 1;

var millisecond = 1;

var second = seconds;

var minute = minutes;

var messages = 1;

var message = 1;

exports.unsafeDecoder = unsafeDecoder$1;
exports.unsafeEncoder = unsafeEncoder$1;
exports.Interop = Interop;
exports.ActorPath = ActorPath;
exports.useStatefulSupervisionPolicy = useStatefulSupervisionPolicy;
exports.spawn = spawn;
exports.spawnStateless = spawnStateless;
exports.spawnPersistent = spawnPersistent;
exports.spawnAdapter = spawnAdapter;
exports.start = start;
exports.stop = stop;
exports.dispatch = dispatch$1;
exports.nobody = nobody;
exports.QueryTimeout = QueryTimeout;
exports.query = query;
exports.milliseconds = milliseconds;
exports.millisecond = millisecond;
exports.seconds = seconds;
exports.second = second;
exports.minutes = minutes;
exports.minute = minute;
exports.hours = hours;
exports.messages = messages;
exports.message = message;
exports.Operators = Operators;
/*  Not a pure module */
