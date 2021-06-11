type actorPath = {
  .
  "parts": array(string),
  "system": string,
};

type observable;

type persistedEvent = {
  .
  "data": Js.Json.t,
  "sequenceNumber": int,
  "key": string,
  "createdAt": int,
  "tags": array(string),
};

type persistedSnapshot = {
  .
  "data": Js.Json.t,
  "sequenceNumber": int,
  "key": string,
  "createdAt": int,
};

type persistenceEngine = {
  .
  "events": (string, int, int, array(string)) => observable,
  "persist": persistedEvent => Js.Promise.t(unit),
  "takeSnapshot": persistedSnapshot => Js.Promise.t(unit),
  "latestSnapshot": string => Js.Promise.t(persistedSnapshot),
};

type actorRef = {
  .
  "parent": actorRef,
  "path": actorPath,
  "name": string,
};

type ctx = {
  .
  "parent": actorRef,
  "path": actorPath,
  "self": actorRef,
  "name": string,
  "children": Nact_jsMap.t(string, actorRef),
  "sender": Js.nullable(actorRef),
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
  "sender": Js.nullable(actorRef),
};

type statefulActor('state, 'msgType) =
  ('state, 'msgType, ctx) => Js.Promise.t('state);

type statelessActor('msgType) = ('msgType, ctx) => Js.Promise.t(unit);

type persistentActor('msg, 'state) =
  ('state, 'msg, persistentCtx('msg)) => Js.Promise.t('state);

type persistentQueryFunc('msg, 'state) =
  ('state, 'msg) => Js.Promise.t('state);

type persistentQuery('state) = unit => Js.Promise.t('state);

type supervisionAction;

type supervisionCtx = {
  .
  "parent": actorRef,
  "path": actorPath,
  "self": actorRef,
  "name": string,
  "children": Nact_jsMap.t(string, actorRef),
  "stop": supervisionAction,
  "stopAll": supervisionAction,
  "escalate": supervisionAction,
  "reset": supervisionAction,
  "resetAll": supervisionAction,
  "resume": supervisionAction,
  "sender": Js.nullable(actorRef),
};

type supervisionFunction('msg, 'parentMsg) =
  ('msg, exn, supervisionCtx) => Js.Promise.t(supervisionAction);

type actorOptions('msg, 'parentMsg, 'state) = {
  .
  "initialStateFunc": Js.Nullable.t((. ctx) => 'state),
  "shutdownAfter": Js.Nullable.t(int),
  "onCrash": Js.Nullable.t(supervisionFunction('msg, 'parentMsg)),
};

type persistentActorOptions('msg, 'parentMsg, 'state) = {
  .
  "initialStateFunc": (. persistentCtx('msg)) => 'state,
  "shutdownAfter": Js.Nullable.t(int),
  "snapshotEvery": Js.Nullable.t(int),
  "onCrash": Js.Nullable.t(supervisionFunction('msg, 'parentMsg)),
  "decoder": Js.Json.t => 'msg,
  "encoder": 'msg => Js.Json.t,
  "snapshotEncoder": 'state => Js.Json.t,
  "snapshotDecoder": Js.Json.t => 'state,
};

type persistentQueryOptions('msg, 'state) = {
  .
  "initialState": 'state,
  "cacheDuration": Js.Nullable.t(int),
  "snapshotEvery": Js.Nullable.t(int),
  "decoder": Js.Json.t => 'msg,
  "snapshotKey": Js.Nullable.t(string),
  "encoder": 'msg => Js.Json.t,
  "snapshotEncoder": 'state => Js.Json.t,
  "snapshotDecoder": Js.Json.t => 'state,
};

[@bs.module "nact"]
external spawn :
  (
    actorRef,
    statefulActor('state, 'msgType),
    Js.nullable(string),
    actorOptions('msgType, 'parentMsg, 'state)
  ) =>
  actorRef =
  "spawn";

[@bs.module "nact"]
external spawnStateless :
  (
    actorRef,
    statelessActor('msgType),
    Js.nullable(string),
    actorOptions('msgType, 'parentMsg, unit)
  ) =>
  actorRef =
  "spawnStateless";

type actor;

[@bs.module "nact/lib/references"] [@bs.new]
external nobody : unit => actorRef = "Nobody";

[@bs.module "nact/lib/actor"] [@bs.val "Actor"] external actor : actor = "";

[@bs.module "nact"]
external spawnPersistent :
  (
    actorRef,
    persistentActor('msgType, 'state),
    string,
    Js.nullable(string),
    persistentActorOptions('msgType, 'parentMsg, 'state)
  ) =>
  actorRef =
  "spawnPersistent";

[@bs.module "nact"]
external persistentQuery :
  (
    actorRef,
    persistentQueryFunc('msgType, 'state),
    string,
    persistentQueryOptions('msgType, 'state)
  ) =>
  persistentQuery('state) =
  "persistentQuery";

type plugin = actorRef => unit;

[@bs.module "nact"]
external configurePersistence : persistenceEngine => plugin = "configurePersistence";

[@bs.module "nact"] external stop : actorRef => unit = "stop";

[@bs.module "nact"] [@bs.splice]
external start : array(plugin) => actorRef = "start";

[@bs.module "nact"] external dispatch : (actorRef, 'msgType) => unit = "dispatch";

[@bs.module "nact"]
external dispatchWithSender :
  (actorRef, 'msgType, Js.nullable(actorRef)) => unit =
  "dispatch";

[@bs.module "nact"]
external query :
  (actorRef, actorRef => 'msgType, int) => Js.Promise.t('expectedResult) =
  "query";
