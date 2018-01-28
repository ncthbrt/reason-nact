type actorPath = {. "parts": list(string), "system": string};

type observable;

type persistedEvent = {
  .
  "data": Js.Json.t, "sequenceNumber": int, "key": string, "createdAt": int, "tags": array(string)
};

type persistedSnapshot = {
  .
  "data": Js.Json.t, "sequenceNumber": int, "key": string, "createdAt": int
};

type persistenceEngine = {
  .
  "events": (string, int, int, array(string)) => observable,
  "persist": persistedEvent => Js.Promise.t(unit),
  "takeSnapshot": persistedSnapshot => Js.Promise.t(unit),
  "latestSnapshot": string => Js.Promise.t(persistedSnapshot)
};

type actorRef = {. "parent": actorRef, "path": actorPath, "name": string};

module Log = {
  type logger;
  type msg = {
    .
    "_type": Js.nullable(string),
    "level": Js.nullable(int),
    "message": Js.nullable(string),
    "name": Js.nullable(string),
    "properties": Js.nullable(Js.Json.t),
    "values": Js.nullable(Js.Json.t),
    "_exception": Js.nullable(exn),
    "actor": Js.nullable(actorRef),
    "createdAt": Js.nullable(Js.Date.t)
  };
  [@bs.send] external trace : (logger, string) => unit = "";
  [@bs.send] external debug : (logger, string) => unit = "";
  [@bs.send] external info : (logger, string) => unit = "";
  [@bs.send] external warn : (logger, string) => unit = "";
  [@bs.send] external error : (logger, string) => unit = "";
  [@bs.send] external critical : (logger, string) => unit = "";
  [@bs.send] external event : (logger, string, 'properties) => unit = "";
  [@bs.send] external metric : (logger, string, 'values) => unit = "";
  [@bs.send "exception"] external exception_ : (logger, exn) => unit = "";
};

type ctx = {
  .
  "sender": Js.nullable(actorRef),
  "parent": actorRef,
  "path": actorPath,
  "self": actorRef,
  "name": string,
  "children": Nact_jsMap.t(string, actorRef),
  "log": Log.logger
};

type persistentCtx('msg) = {
  .
  "parent": actorRef,
  "path": actorPath,
  "self": actorRef,
  "name": string,
  "children": Nact_jsMap.t(string, actorRef),
  "persist": 'msg => Js.Promise.t(unit),
  "recovering": Js.Nullable.t(bool),
  "log": Log.logger
};

type statefulActor('state, 'msgType) =
  (Js.nullable('state), 'msgType, ctx) => Js.Promise.t('state);

type statelessActor('msgType) = ('msgType, ctx) => Js.Promise.t(unit);

type persistentActor('msgType) =
  (Js.nullable(Js.Json.t), Js.Json.t, persistentCtx('msgType)) => Js.Promise.t(Js.Json.t);

type supervisionAction;

type supervisionCtx = {
  .
  "sender": Js.nullable(actorRef),
  "parent": actorRef,
  "path": actorPath,
  "self": actorRef,
  "name": string,
  "child": actorRef,
  "children": Nact_jsMap.t(string, actorRef),
  "stop": supervisionAction,
  "stopAll": supervisionAction,
  "escalate": supervisionAction,
  "reset": supervisionAction,
  "resetAll": supervisionAction,
  "resume": supervisionAction
};

type supervisionFunction = (unit, exn, supervisionCtx) => Js.Promise.t(supervisionAction);

type actorOptions = {
  .
  "shutdownAfter": Js.Nullable.t(int), "whenChildCrashes": Js.Nullable.t(supervisionFunction)
};

type persistentActorOptions = {
  .
  "shutdownAfter": Js.Nullable.t(int),
  "snapshotEvery": Js.Nullable.t(int),
  "whenChildCrashes": Js.Nullable.t(supervisionFunction)
};

[@bs.module "nact"]
external spawn :
  (actorRef, statefulActor('state, 'msgType), Js.nullable(string), actorOptions) => actorRef =
  "";

[@bs.module "nact"]
external spawnStateless :
  (actorRef, statelessActor('msgType), Js.nullable(string), actorOptions) => actorRef =
  "";

type actor;

[@bs.module "nact/lib/references"] [@bs.new] external nobody : unit => actorRef = "Nobody";

[@bs.module "nact/lib/actor"] [@bs.val "Actor"] external actor : actor = "";

[@bs.module "nact"]
external spawnPersistent :
  (actorRef, persistentActor('msgType), string, Js.nullable(string), persistentActorOptions) =>
  actorRef =
  "";

type plugin = actorRef => unit;

[@bs.module "nact"] external configurePersistence : persistenceEngine => plugin = "";

[@bs.module "nact"] external configureLogging : (actorRef => actorRef) => plugin = "";

[@bs.module "nact"] external stop : actorRef => unit = "";

[@bs.module "nact"] [@bs.splice] external start : array(plugin) => actorRef = "";

[@bs.module "nact"] external dispatch : (actorRef, 'msgType) => unit = "";

[@bs.module "nact"]
external query : (actorRef, actorRef => 'msgType, int) => Js.Promise.t('expectedResult) =
  "";