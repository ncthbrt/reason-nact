module StringSet = Nact_stringSet;

type actorPath;

type persistenceEngine;

type actorRef('msg);

type ctx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  children: StringSet.t,
  name: string
};

type persistentCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  path: actorPath,
  self: actorRef('msg),
  name: string,
  persist: 'msg => Js.Promise.t(unit),
  children: StringSet.t,
  recovering: bool
};

type supervisionCtx('msg, 'parentMsg) = {
  parent: actorRef('parentMsg),
  child: string,
  path: actorPath,
  self: actorRef('msg),
  name: string,
  children: StringSet.t
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

type statelessActor('msg, 'parentMsg) = ('msg, ctx('msg, 'parentMsg)) => Js.Promise.t(unit);

type persistentActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state);

type supervisionPolicy('msg, 'parentMsg) =
  (exn, supervisionCtx('msg, 'parentMsg)) => Js.Promise.t(supervisionAction);

type statefulSupervisionPolicy('msg, 'parentMsg, 'state) =
  (exn, 'state, supervisionCtx('msg, 'parentMsg)) => ('state, Js.Promise.t(supervisionAction));

let useStatefulSupervisionPolicy:
  (statefulSupervisionPolicy('msg, 'parentMsg, 'state), 'state) =>
  supervisionPolicy('msg, 'parentMsg);

let spawn:
  (
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~whenChildCrashes: supervisionPolicy('msg, 'parentMsg)=?,
    actorRef('parentMsg),
    statefulActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg);

let spawnStateless:
  (
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~whenChildCrashes: supervisionPolicy('msg, 'parentMsg)=?,
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
    ~whenChildCrashes: supervisionPolicy('msg, 'parentMsg)=?,
    actorRef('parentMsg),
    persistentActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg);

let stop: actorRef('msg) => unit;

type systemMsg;

let start: (~persistenceEngine: persistenceEngine=?, unit) => actorRef(systemMsg);

let dispatch: (actorRef('msg), 'msg) => unit;

exception QueryTimeout(int);

let query: (~timeout: int, actorRef('msg), actorRef('outgoing) => 'msg) => Js.Promise.t('outgoing);

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
  let (<?): (actorRef('msg), (actorRef('outgoing) => 'msg, int)) => Js.Promise.t('outgoing);
};