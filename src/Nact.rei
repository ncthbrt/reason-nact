type persistenceEngine;

type systemMsg;

type actorRef('msg);

type untypedRef;

type actorPath;

module Interop: {
  let fromUntypedRef: untypedRef => actorRef('msg);
  let toUntypedRef: actorRef('msg) => untypedRef;
  let dispatch: (untypedRef, 'msg) => unit;
};

module ActorPath: {
  let fromReference: actorRef(_) => actorPath;
  let systemName: actorPath => string;
  let toString: actorPath => string;
  let parts: actorPath => list(string);
};

module Log: {
  /* logEngine is an opaque type which dispatches messages to the logging actor */
  type loggingEngine;
  type name = string;
  type logLevel =
    | Off
    | Trace
    | Debug
    | Info
    | Warn
    | Error
    | Critical;
  let logLevelToString: logLevel => string;
  type t =
    | Message(logLevel, string, Js.Date.t, actorPath)
    | Error(exn, Js.Date.t, actorPath)
    | Metric(name, Js.Json.t, Js.Date.t, actorPath)
    | Event(name, Js.Json.t, Js.Date.t, actorPath)
    | Unknown(Js.Json.t, Js.Date.t, actorPath);
  type logger = actorRef(systemMsg) => actorRef(t);
  let trace: (string, loggingEngine) => unit;
  let debug: (string, loggingEngine) => unit;
  let info: (string, loggingEngine) => unit;
  let warn: (string, loggingEngine) => unit;
  let error: (string, loggingEngine) => unit;
  let critical: (string, loggingEngine) => unit;
  let event:
    (~name: string, ~properties: 'eventProperties, loggingEngine) => unit;
  let exception_: (exn, loggingEngine) => unit;
  let metric: (~name: string, ~values: 'values, loggingEngine) => unit;
};

type ctx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  children: Belt.Set.String.t,
  name: string,
  logger: Log.loggingEngine,
};

type persistentCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  name: string,
  persist: 'msg => Js.Promise.t(unit),
  children: Belt.Set.String.t,
  recovering: bool,
  logger: Log.loggingEngine,
};

type supervisionCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  name: string,
  children: Belt.Set.String.t,
};

type supervisionAction =
  | Stop
  | StopAll
  | Reset
  | ResetAll
  | Escalate
  | Resume;

type statefulActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, ctx('msg, 'parentMsg)) => Js.Promise.t('state);

type statelessActor('msg, 'parentMsg) =
  ('msg, ctx('msg, 'parentMsg)) => Js.Promise.t(unit);

type persistentActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state);

type supervisionPolicy('msg, 'parentMsg) =
  ('msg, exn, supervisionCtx('msg, 'parentMsg)) =>
  Js.Promise.t(supervisionAction);

type statefulSupervisionPolicy('msg, 'parentMsg, 'state) =
  ('msg, exn, 'state, supervisionCtx('msg, 'parentMsg)) =>
  ('state, Js.Promise.t(supervisionAction));

let useStatefulSupervisionPolicy:
  (statefulSupervisionPolicy('msg, 'parentMsg, 'state), 'state) =>
  supervisionPolicy('msg, 'parentMsg);

let spawn:
  (
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~onCrash: supervisionPolicy('msg, 'parentMsg)=?,
    actorRef('parentMsg),
    statefulActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg);

let spawnStateless:
  (
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~onCrash: supervisionPolicy('msg, 'parentMsg)=?,
    actorRef('parentMsg),
    statelessActor('msg, 'parentMsg)
  ) =>
  actorRef('msg);

let spawnPersistent:
  (
    ~key: string,
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~snapshotEvery: int=?,
    ~onCrash: supervisionPolicy('msg, 'parentMsg)=?,
    ~decoder: Js.Json.t => 'msg=?,
    ~stateDecoder: Js.Json.t => 'state=?,
    ~stateEncoder: 'state => Js.Json.t=?,
    ~encoder: 'msg => Js.Json.t=?,
    actorRef('parentMsg),
    persistentActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg);

let spawnAdapter:
  (actorRef('parentMsg), 'msg => 'parentMsg) => actorRef('msg);

let start:
  (
    ~name: string=?,
    ~persistenceEngine: persistenceEngine=?,
    ~logger: Log.logger=?,
    unit
  ) =>
  actorRef(systemMsg);

let stop: actorRef('msg) => unit;

let dispatch: (actorRef('msg), 'msg) => unit;

let nobody: unit => actorRef('a);

exception QueryTimeout(int);

let query:
  (~timeout: int, actorRef('msg), actorRef('outgoing) => 'msg) =>
  Js.Promise.t('outgoing);

let milliseconds: int;

let millisecond: int;

let seconds: int;

let second: int;

let minutes: int;

let minute: int;

let hours: int;

let messages: int;

let message: int;

module Operators: {
  let (<-<): (actorRef('a), 'a) => unit;
  let (>->): ('a, actorRef('a)) => unit;
  let (<?):
    (actorRef('msg), (actorRef('outgoing) => 'msg, int)) =>
    Js.Promise.t('outgoing);
};