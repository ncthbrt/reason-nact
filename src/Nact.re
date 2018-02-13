module StringSet = Nact_stringSet;

open Js.Promise;

open Js.Nullable;

let defaultTo = (default, opt) =>
  switch opt {
  | Some(opt) => opt
  | None => default
  };

type persistenceEngine = Nact_bindings.persistenceEngine;

type actorRef('msg) =
  | ActorRef(Nact_bindings.actorRef);

type actorPath =
  | ActorPath(Nact_bindings.actorPath);

module ActorPath = {
  let fromReference = (ActorRef(actor)) => ActorPath(actor##path);
  let systemName = (ActorPath(path)) => path##system;
  let toString = (ActorPath(path)) =>
    "system:" ++ path##system ++ "//" ++ String.concat("/", path##parts);
};

type systemMsg;

[%%bs.raw
  {|
/* This code is to handle how bucklescript sometimes represents variants */
 
var WrappedVariant = '_wvariant';
var WrappedEvent = '_wevent';
function unsafeEncoder(obj) {  
  var serialized = JSON.stringify(obj, function (key, value) {
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
  return { serialized, type: WrappedEvent };
};

function unsafeDecoder(result) {
  if(result && typeof(result) === 'object' && result.type === WrappedEvent) {
    return JSON.parse(result.serialized, (key, value) => {
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
|}
];

[@bs.val] external unsafeDecoder : Js.Json.t => 'msg = "unsafeDecoder";

[@bs.val] external unsafeEncoder : 'msg => Js.Json.t = "unsafeEncoder";

module Log = {
  type loggingEngine = Nact_bindings.Log.logger;
  type name = string;
  [@bs.deriving jsConverter]
  type logLevel =
    | [@bs.as 0] Off
    | [@bs.as 1] Trace
    | [@bs.as 2] Debug
    | [@bs.as 3] Info
    | [@bs.as 4] Warn
    | [@bs.as 5] Error
    | [@bs.as 6] Critical;
  let logLevelToString =
    fun
    | Off => "off"
    | Trace => "trace"
    | Debug => "debug"
    | Info => "info"
    | Warn => "warn"
    | Error => "error"
    | Critical => "critical";
  type t =
    | Message(logLevel, string, Js.Date.t, actorPath)
    | Error(exn, Js.Date.t, actorPath)
    | Metric(name, Js.Json.t, Js.Date.t, actorPath)
    | Event(name, Js.Json.t, Js.Date.t, actorPath)
    | Unknown(Js.Json.t, Js.Date.t, actorPath);
  type jsLog = Nact_bindings.Log.msg;
  let fromJsLog: jsLog => t =
    msg => {
      let path =
        msg##actor
        |> Js.Nullable.to_opt
        |> defaultTo(Nact_bindings.nobody())
        |> (a => ActorPath(a##path));
      let createdAt =
        msg##createdAt |> Js.Nullable.to_opt |> defaultTo(Js.Date.make());
      switch (Js.Nullable.to_opt(msg##_type)) {
      | Some("trace") =>
        let result =
          Message(
            msg##level
            |> Js.Nullable.to_opt
            |> defaultTo(0)
            |> logLevelFromJs
            |> defaultTo(Off),
            msg##message |> Js.Nullable.to_opt |> defaultTo(""),
            createdAt,
            path
          );
        result;
      | Some("metric") =>
        Metric(
          msg##name |> to_opt |> defaultTo(""),
          msg##values |> to_opt |> defaultTo(Js.Json.null),
          createdAt,
          path
        )
      | Some("event") =>
        Event(
          msg##name |> to_opt |> defaultTo(""),
          msg##properties |> to_opt |> defaultTo(Js.Json.null),
          createdAt,
          path
        )
      | Some("exception") =>
        Error(
          msg##_exception |> to_opt |> defaultTo(Failure("Error is undefined")),
          createdAt,
          path
        )
      | _ => Unknown(msg |> unsafeEncoder, createdAt, path)
      };
    };
  type logger = actorRef(systemMsg) => actorRef(t);
  let trace = (message, loggingEngine) =>
    Nact_bindings.Log.trace(loggingEngine, message);
  let debug = (message, loggingEngine) =>
    Nact_bindings.Log.debug(loggingEngine, message);
  let info = (message, loggingEngine) =>
    Nact_bindings.Log.info(loggingEngine, message);
  let warn = (message, loggingEngine) =>
    Nact_bindings.Log.warn(loggingEngine, message);
  let error = (message, loggingEngine) =>
    Nact_bindings.Log.error(loggingEngine, message);
  let critical = (message, loggingEngine) =>
    Nact_bindings.Log.critical(loggingEngine, message);
  let event = (~name, ~properties, loggingEngine) =>
    Nact_bindings.Log.event(loggingEngine, name, properties);
  let metric = (~name, ~values, loggingEngine) =>
    Nact_bindings.Log.metric(loggingEngine, name, values);
  let exception_ = (err, loggingEngine) =>
    Nact_bindings.Log.exception_(loggingEngine, err);
};

type ctx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  children: StringSet.t,
  name: string,
  logger: Log.loggingEngine
};

type persistentCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  name: string,
  persist: 'msg => Js.Promise.t(unit),
  children: StringSet.t,
  recovering: bool,
  logger: Log.loggingEngine
};

let mapCtx = (untypedCtx: Nact_bindings.ctx) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  path: ActorPath(untypedCtx##path),
  children: untypedCtx##children |> Nact_jsMap.keys |> StringSet.fromJsArray,
  logger: untypedCtx##log
};

let mapPersist = (encoder, persist, msg) => persist(encoder(msg));

let mapPersistentCtx =
    (untypedCtx: Nact_bindings.persistentCtx('incoming), encoder) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  path: ActorPath(untypedCtx##path),
  recovering: untypedCtx##recovering |> Js.Nullable.to_opt |> defaultTo(false),
  persist: mapPersist(encoder, untypedCtx##persist),
  children: untypedCtx##children |> Nact_jsMap.keys |> StringSet.fromJsArray,
  logger: untypedCtx##log
};

type supervisionCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  child: string,
  path: actorPath,
  self: actorRef('msg),
  name: string,
  children: StringSet.t
};

let mapSupervisionCtx = (untypedCtx: Nact_bindings.supervisionCtx) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  path: ActorPath(untypedCtx##path),
  children: untypedCtx##children |> Nact_jsMap.keys |> StringSet.fromJsArray,
  child: untypedCtx##child##name
};

type supervisionAction =
  | Stop
  | StopAll
  | Reset
  | ResetAll
  | Escalate
  | Resume;

type supervisionPolicy('msg, 'parentMsg) =
  (exn, supervisionCtx('msg, 'parentMsg)) => Js.Promise.t(supervisionAction);

type statefulSupervisionPolicy('msg, 'parentMsg, 'state) =
  (exn, 'state, supervisionCtx('msg, 'parentMsg)) =>
  ('state, Js.Promise.t(supervisionAction));

let mapSupervisionFunction = optionalF =>
  switch optionalF {
  | None => Js.Nullable.undefined
  | Some(f) =>
    Js.Nullable.return((_, err, ctx) =>
      f(err, mapSupervisionCtx(ctx))
      |> then_(decision =>
           resolve(
             switch decision {
             | Stop => ctx##stop
             | StopAll => ctx##stopAll
             | Reset => ctx##reset
             | ResetAll => ctx##resetAll
             | Escalate => ctx##escalate
             | Resume => ctx##resume
             }
           )
         )
    )
  };

type statefulActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, ctx('msg, 'parentMsg)) => Js.Promise.t('state);

type statelessActor('msg, 'parentMsg) =
  ('msg, ctx('msg, 'parentMsg)) => Js.Promise.t(unit);

type persistentActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state);

let useStatefulSupervisionPolicy = (f, initialState) => {
  let state = ref(initialState);
  (err, ctx) => {
    let (nextState, promise) = f(err, state^, ctx);
    state := nextState;
    promise;
  };
};

let spawn =
    (
      ~name=?,
      ~shutdownAfter=?,
      ~whenChildCrashes=?,
      ActorRef(parent),
      func,
      initialState
    ) => {
  let options = {
    "shutdownAfter": from_opt(shutdownAfter),
    "whenChildCrashes": mapSupervisionFunction(whenChildCrashes)
  };
  let f = (possibleState: Js.nullable('state), msg: 'msg, ctx) => {
    let state = Js.Nullable.to_opt(possibleState) |> defaultTo(initialState);
    try (func(state, msg, mapCtx(ctx))) {
    | err => reject(err)
    };
  };
  let untypedRef = Nact_bindings.spawn(parent, f, from_opt(name), options);
  ActorRef(untypedRef);
};

let spawnStateless =
    (~name=?, ~shutdownAfter=?, ~whenChildCrashes=?, ActorRef(parent), func) => {
  let options = {
    "shutdownAfter": from_opt(shutdownAfter),
    "whenChildCrashes": mapSupervisionFunction(whenChildCrashes)
  };
  let f = (msg, ctx) =>
    try (func(msg, mapCtx(ctx))) {
    | _ => Js.Promise.resolve()
    };
  let untypedRef =
    Nact_bindings.spawnStateless(parent, f, from_opt(name), options);
  ActorRef(untypedRef);
};

let spawnPersistent =
    (
      ~key,
      ~name=?,
      ~shutdownAfter=?,
      ~snapshotEvery=?,
      ~whenChildCrashes=?,
      ~decoder=?,
      ~stateDecoder=?,
      ~stateEncoder=?,
      ~encoder=?,
      ActorRef(parent),
      func:
        ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state),
      initialState: 'state
    ) => {
  let decoder = decoder |> defaultTo(unsafeDecoder);
  let stateDecoder = stateDecoder |> defaultTo(unsafeDecoder);
  let stateEncoder = stateEncoder |> defaultTo(unsafeEncoder);
  let encoder = encoder |> defaultTo(unsafeEncoder);
  let options: Nact_bindings.persistentActorOptions = {
    "shutdownAfter": from_opt(shutdownAfter),
    "snapshotEvery": from_opt(snapshotEvery),
    "whenChildCrashes": mapSupervisionFunction(whenChildCrashes)
  };
  let f = (state, msg, ctx) => {
    let state =
      switch (Js.Nullable.to_opt(state)) {
      | None => initialState
      | Some(state) => stateDecoder(state)
      };
    (
      try (func(state, decoder(msg), mapPersistentCtx(ctx, encoder))) {
      | err => reject(err)
      }
    )
    |> then_(result => resolve(stateEncoder(result)));
  };
  let untypedRef =
    Nact_bindings.spawnPersistent(parent, f, key, from_opt(name), options);
  ActorRef(untypedRef);
};

let stop = (ActorRef(reference)) => Nact_bindings.stop(reference);

let dispatch = (ActorRef(recipient), msg) =>
  Nact_bindings.dispatch(recipient, msg);

let nobody = () => ActorRef(Nact_bindings.nobody());

let spawnAdapter = (parent, mapping) =>
  spawnStateless(parent, (msg, _) => resolve(dispatch(parent, mapping(msg))));

let mapLoggingActor = (loggingActorFunction: Log.logger, system) => {
  let loggerActor = loggingActorFunction(ActorRef(system));
  let ActorRef(adapter) = spawnAdapter(loggerActor, Log.fromJsLog);
  adapter;
};

let start = (~persistenceEngine=?, ~logger=?, ()) => {
  let untypedRef =
    switch (persistenceEngine, logger) {
    | (Some(persistence), Some(logger)) =>
      Nact_bindings.start([|
        Nact_bindings.configurePersistence(persistence),
        Nact_bindings.configureLogging(mapLoggingActor(logger))
      |])
    | (None, Some(logger)) =>
      Nact_bindings.start([|
        Nact_bindings.configureLogging(mapLoggingActor(logger))
      |])
    | (Some(persistence), None) =>
      Nact_bindings.start([|Nact_bindings.configurePersistence(persistence)|])
    | (None, None) => Nact_bindings.start([||])
    };
  ActorRef(untypedRef);
};

exception QueryTimeout(int);

let query = (~timeout: int, ActorRef(recipient), msgF) => {
  let f = tempReference => msgF(ActorRef(tempReference));
  Nact_bindings.query(recipient, f, timeout)
  |> catch((_) => reject(QueryTimeout(timeout)));
};

let milliseconds = 1;

let millisecond = milliseconds;

let seconds = 1000 * milliseconds;

let second = seconds;

let minutes = 60 * seconds;

let minute = minutes;

let hours = 60 * minutes;

let messages = 1;

let message = 1;

module Operators = {
  let (<-<) = (actorRef, msg) => dispatch(actorRef, msg);
  let (>->) = (msg, actorRef) => dispatch(actorRef, msg);
  let (<?) = (actor, (f, timeout)) => query(~timeout, actor, f);
};