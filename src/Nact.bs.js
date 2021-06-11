'use strict';

var Nact = require("nact");
var Curry = require("bs-platform/lib/js/curry.js");
var $$String = require("bs-platform/lib/js/string.js");
var Belt_List = require("bs-platform/lib/js/belt_List.js");
var Nact_jsMap = require("./Nact_jsMap.bs.js");
var Belt_Option = require("bs-platform/lib/js/belt_Option.js");
var Caml_option = require("bs-platform/lib/js/caml_option.js");
var Belt_SetString = require("bs-platform/lib/js/belt_SetString.js");
var Caml_exceptions = require("bs-platform/lib/js/caml_exceptions.js");
var Js_null_undefined = require("bs-platform/lib/js/js_null_undefined.js");
var Caml_js_exceptions = require("bs-platform/lib/js/caml_js_exceptions.js");
var References = require("nact/lib/references");

function fromUntypedRef(reference) {
  return /* ActorRef */{
          _0: reference
        };
}

function toUntypedRef(reference) {
  return reference._0;
}

function dispatch(prim, prim$1) {
  Nact.dispatch(prim, prim$1);
  
}

function dispatchWithSender(prim, prim$1, prim$2) {
  Nact.dispatch(prim, prim$1, prim$2);
  
}

var Interop = {
  fromUntypedRef: fromUntypedRef,
  toUntypedRef: toUntypedRef,
  dispatch: dispatch,
  dispatchWithSender: dispatchWithSender
};

function fromReference(actor) {
  return /* ActorPath */{
          _0: actor._0.path
        };
}

function systemName(path) {
  return path._0.system;
}

function toString(path) {
  var path$1 = path._0;
  return "system:" + (path$1.system + ("//" + $$String.concat("/", Belt_List.fromArray(path$1.parts))));
}

function parts(path) {
  return Belt_List.fromArray(path._0.parts);
}

var ActorPath = {
  fromReference: fromReference,
  systemName: systemName,
  toString: toString,
  parts: parts
};

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
  return {
          parent: /* ActorRef */{
            _0: untypedCtx.parent
          },
          path: /* ActorPath */{
            _0: untypedCtx.path
          },
          self: /* ActorRef */{
            _0: untypedCtx.self
          },
          children: Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children)),
          name: untypedCtx.name,
          sender: untypedCtx.sender
        };
}

function mapPersistentCtx(untypedCtx) {
  return {
          parent: /* ActorRef */{
            _0: untypedCtx.parent
          },
          path: /* ActorPath */{
            _0: untypedCtx.path
          },
          self: /* ActorRef */{
            _0: untypedCtx.self
          },
          name: untypedCtx.name,
          persist: untypedCtx.persist,
          children: Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children)),
          recovering: Belt_Option.getWithDefault(Caml_option.nullable_to_opt(untypedCtx.recovering), false),
          sender: untypedCtx.sender
        };
}

function mapSupervisionCtx(untypedCtx) {
  return {
          parent: /* ActorRef */{
            _0: untypedCtx.parent
          },
          path: /* ActorPath */{
            _0: untypedCtx.path
          },
          self: /* ActorRef */{
            _0: untypedCtx.self
          },
          name: untypedCtx.name,
          children: Belt_SetString.fromArray(Nact_jsMap.keys(untypedCtx.children)),
          sender: untypedCtx.sender
        };
}

function mapSupervisionFunction(optionalF) {
  if (optionalF !== undefined) {
    return function (msg, err, ctx) {
      return Curry._3(optionalF, msg, err, mapSupervisionCtx(ctx)).then(function (decision) {
                  var tmp;
                  switch (decision) {
                    case /* Stop */0 :
                        tmp = ctx.stop;
                        break;
                    case /* StopAll */1 :
                        tmp = ctx.stopAll;
                        break;
                    case /* Reset */2 :
                        tmp = ctx.reset;
                        break;
                    case /* ResetAll */3 :
                        tmp = ctx.resetAll;
                        break;
                    case /* Escalate */4 :
                        tmp = ctx.escalate;
                        break;
                    case /* Resume */5 :
                        tmp = ctx.resume;
                        break;
                    
                  }
                  return Promise.resolve(tmp);
                });
    };
  }
  
}

function useStatefulSupervisionPolicy(f, initialState) {
  var state = {
    contents: initialState
  };
  return function (msg, err, ctx) {
    var match = Curry._4(f, msg, err, state.contents, ctx);
    state.contents = match[0];
    return match[1];
  };
}

function spawn(name, shutdownAfter, onCrash, parent, func, initialState) {
  var options = {
    initialStateFunc: (function (ctx) {
        return Curry._1(initialState, mapCtx(ctx));
      }),
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    onCrash: mapSupervisionFunction(onCrash)
  };
  var f = function (state, msg, ctx) {
    try {
      return Curry._3(func, state, msg, mapCtx(ctx));
    }
    catch (raw_err){
      return Promise.reject(Caml_js_exceptions.internalToOCamlException(raw_err));
    }
  };
  var untypedRef = Nact.spawn(parent._0, f, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */{
          _0: untypedRef
        };
}

function spawnStateless(name, shutdownAfter, parent, func) {
  var options = {
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    initialStateFunc: undefined,
    onCrash: mapSupervisionFunction(undefined)
  };
  var f = function (msg, ctx) {
    try {
      return Curry._2(func, msg, mapCtx(ctx));
    }
    catch (raw_e){
      return Promise.reject(Caml_js_exceptions.internalToOCamlException(raw_e));
    }
  };
  var untypedRef = Nact.spawnStateless(parent._0, f, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */{
          _0: untypedRef
        };
}

function spawnPersistent(key, name, shutdownAfter, snapshotEvery, onCrash, decoder, stateDecoder, encoder, stateEncoder, parent, func, initialState) {
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
    initialStateFunc: (function (ctx) {
        return Curry._1(initialState, mapPersistentCtx(ctx));
      }),
    shutdownAfter: Js_null_undefined.fromOption(shutdownAfter),
    onCrash: mapSupervisionFunction(onCrash),
    snapshotEvery: Js_null_undefined.fromOption(snapshotEvery),
    encoder: encoder$1,
    decoder: decoder$1,
    snapshotEncoder: stateEncoder$1,
    snapshotDecoder: stateDecoder$1
  };
  var f = function (state, msg, ctx) {
    try {
      return Curry._3(func, state, msg, mapPersistentCtx(ctx));
    }
    catch (raw_err){
      return Promise.reject(Caml_js_exceptions.internalToOCamlException(raw_err));
    }
  };
  var untypedRef = Nact.spawnPersistent(parent._0, f, key, Js_null_undefined.fromOption(name), options);
  return /* ActorRef */{
          _0: untypedRef
        };
}

function persistentQuery(key, snapshotKey, cacheDuration, snapshotEvery, decoder, stateDecoder, encoder, stateEncoder, actor, func, initialState) {
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
    initialState: initialState,
    cacheDuration: Js_null_undefined.fromOption(cacheDuration),
    snapshotEvery: Js_null_undefined.fromOption(snapshotEvery),
    snapshotKey: Js_null_undefined.fromOption(snapshotKey),
    encoder: encoder$1,
    decoder: decoder$1,
    snapshotEncoder: stateEncoder$1,
    snapshotDecoder: stateDecoder$1
  };
  var f = function (state, msg) {
    try {
      return Curry._2(func, state, msg);
    }
    catch (raw_err){
      return Promise.reject(Caml_js_exceptions.internalToOCamlException(raw_err));
    }
  };
  return Nact.persistentQuery(actor._0, f, key, options);
}

function stop(reference) {
  Nact.stop(reference._0);
  
}

function dispatch$1(recipient, msg) {
  Nact.dispatch(recipient._0, msg);
  
}

function nobody(param) {
  return /* ActorRef */{
          _0: new References.Nobody()
        };
}

function spawnAdapter(name, parent, mapping) {
  var f = function (msg, param) {
    return Promise.resolve(dispatch$1(parent, Curry._1(mapping, msg)));
  };
  if (name !== undefined) {
    return spawnStateless(name, undefined, parent, f);
  } else {
    return spawnStateless(undefined, undefined, parent, f);
  }
}

function start(name, persistenceEngine, param) {
  var plugins = persistenceEngine !== undefined ? ({
        hd: Nact.configurePersistence(Caml_option.valFromOption(persistenceEngine)),
        tl: /* [] */0
      }) : /* [] */0;
  var plugins$1 = name !== undefined ? ({
        hd: {
          name: name
        },
        tl: plugins
      }) : plugins;
  if (!plugins$1) {
    return /* ActorRef */{
            _0: Nact.start()
          };
  }
  var match = plugins$1.tl;
  var a = plugins$1.hd;
  if (match) {
    return /* ActorRef */{
            _0: Nact.start(a, match.hd)
          };
  } else {
    return /* ActorRef */{
            _0: Nact.start(a)
          };
  }
}

var QueryTimeout = /* @__PURE__ */Caml_exceptions.create("Nact.QueryTimeout");

function query(timeout, recipient, msgF) {
  var f = function (tempReference) {
    return Curry._1(msgF, /* ActorRef */{
                _0: tempReference
              });
  };
  return Nact.query(recipient._0, f, timeout).catch(function (param) {
              return Promise.reject({
                          RE_EXN_ID: QueryTimeout,
                          _1: timeout
                        });
            });
}

var seconds = 1000;

var minutes = Math.imul(60, seconds);

var hours = Math.imul(60, minutes);

var $less$neg$less = dispatch$1;

function $great$neg$great(msg, actorRef) {
  return dispatch$1(actorRef, msg);
}

function $less$question(actor, param) {
  return query(param[1], actor, param[0]);
}

var Operators = {
  $less$neg$less: $less$neg$less,
  $great$neg$great: $great$neg$great,
  $less$question: $less$question
};

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
exports.persistentQuery = persistentQuery;
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
