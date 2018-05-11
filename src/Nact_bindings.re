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
};

type statefulActor('state, 'msgType) =
  (Js.nullable('state), 'msgType, ctx) => Js.Promise.t('state);

type statelessActor('msgType) = ('msgType, ctx) => Js.Promise.t(unit);

type persistentActor('msg, 'state) =
  (Js.nullable('state), 'msg, persistentCtx('msg)) => Js.Promise.t('state);

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
};

type supervisionFunction('msg, 'parentMsg) =
  ('msg, exn, supervisionCtx) => Js.Promise.t(supervisionAction);

type actorOptions('msg, 'parentMsg) = {
  .
  "shutdownAfter": Js.Nullable.t(int),
  "onCrash": Js.Nullable.t(supervisionFunction('msg, 'parentMsg)),
};

type persistentActorOptions('msg, 'parentMsg, 'state) = {
  .
  "shutdownAfter": Js.Nullable.t(int),
  "snapshotEvery": Js.Nullable.t(int),
  "onCrash": Js.Nullable.t(supervisionFunction('msg, 'parentMsg)),
  "decoder": Js.Json.t => 'msg,
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
    actorOptions('msgType, 'parentMsg)
  ) =>
  actorRef =
  "";

[@bs.module "nact"]
external spawnStateless :
  (
    actorRef,
    statelessActor('msgType),
    Js.nullable(string),
    actorOptions('msgType, 'parentMsg)
  ) =>
  actorRef =
  "";

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
  "";

type plugin = actorRef => unit;

[@bs.module "nact"]
external configurePersistence : persistenceEngine => plugin = "";

[@bs.module "nact"] external stop : actorRef => unit = "";

[@bs.module "nact"] [@bs.splice]
external start : array(plugin) => actorRef = "";

[@bs.module "nact"] external dispatch : (actorRef, 'msgType) => unit = "";

[@bs.module "nact"]
external query :
  (actorRef, actorRef => 'msgType, int) => Js.Promise.t('expectedResult) =
  "";