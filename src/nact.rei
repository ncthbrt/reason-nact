module StringSet = Nact_stringSet;

type actorPath;

type persistenceEngine;

type reference('msg);

type actorRef('msg) = [ | `ActorRef(reference('msg))];

type clusterRef('msg) = [ | `ClusterRef(reference('msg))];

type dispatchable('msg) = [
  clusterRef('msg) 
  | actorRef('msg)
];

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

module Cluster: {
  type keySelector('msg) = 'msg => int;
  type routingStrategy('msg) =
    | Sharded(keySelector('msg));
  let createCluster: (~name: string, routingStrategy('msg)) => clusterRef('msg);
  let join: (clusterRef('msg), actorRef('msg)) => unit;
  let leave: (clusterRef('msg), actorRef('msg)) => unit;
  module Operators: {
    let (+@): (clusterRef('msg), actorRef('msg)) => unit;
    let (-@): (clusterRef('msg), actorRef('msg)) => unit;
  };
};

let stop: actorRef('a) => unit;

type systemMsg;

let start: (~persistenceEngine: persistenceEngine=?, unit) => actorRef(systemMsg);

let dispatch: ([< | dispatchable('a)], 'msg) => unit;

exception QueryTimeout(int);

let query:
  (~timeout: int, [< | dispatchable('a)], actorRef('outgoing) => 'msg) => Js.Promise.t('outgoing);

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
  let (<-<): ([< | dispatchable('a)], 'a) => unit;
  let (>->): ([< | dispatchable('a)], 'a) => unit;
};