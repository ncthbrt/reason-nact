module StringSet = Nact_stringSet;

type actorPath;

type persistenceEngine;

type actorRef('msg);

type localActorRef('msg) = [ | `LocalRef(actorRef('msg))];

type clusterRef('msg) = [ | `ClusterRef(actorRef('msg))];

type dispatchable('msg) = [ 
  clusterRef('msg) 
  | localActorRef('msg)
];

type ctx('msg, 'parentMsg) = {
  parent: localActorRef('parentMsg),
  path: actorPath,
  self: localActorRef('msg),
  children: StringSet.t,
  name: string
};

type persistentCtx('msg, 'parentMsg) = {
  parent: localActorRef('parentMsg),
  path: actorPath,
  self: localActorRef('msg),
  name: string,
  persist: 'msg => Js.Promise.t(unit),
  children: StringSet.t,
  recovering: bool
};

type supervisionCtx('msg, 'parentMsg) = {
  parent: localActorRef('parentMsg),
  child: string,
  path: actorPath,
  self: localActorRef('msg),
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
    localActorRef('parentMsg),
    statefulActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  localActorRef('msg);

let spawnStateless:
  (
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~whenChildCrashes: supervisionPolicy('msg, 'parentMsg)=?,
    localActorRef('parentMsg),
    statelessActor('msg, 'parentMsg)
  ) =>
  localActorRef('msg);

let spawnPersistent:
  (
    ~key: string,
    ~name: string=?,
    ~shutdownAfter: int=?,
    ~snapshotEvery: int=?,
    ~whenChildCrashes: supervisionPolicy('msg, 'parentMsg)=?,
    localActorRef('parentMsg),
    persistentActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  localActorRef('msg);

module Cluster: {
  type keySelector('msg) = 'msg => int;
  type routingStrategy('msg) =
    | Sharded(keySelector('msg));
  let createCluster: (~name: string, routingStrategy('msg)) => clusterRef('msg);
  let join: (clusterRef('msg), localActorRef('msg)) => unit;
  let leave: (clusterRef('msg), localActorRef('msg)) => unit;
  module Operators: {
    let (+@): (clusterRef('msg), localActorRef('msg)) => unit;
    let (-@): (clusterRef('msg), localActorRef('msg)) => unit;
  };
};

let stop: dispatchable('msg) => unit;

type systemMsg;

let start: (~persistenceEngine: persistenceEngine=?, unit) => localActorRef(systemMsg);

let dispatch: (dispatchable('msg), 'msg) => unit;

exception QueryTimeout(int);

let query:
  (~timeout: int, dispatchable('msg), localActorRef('outgoing) => 'msg) => Js.Promise.t('outgoing);

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
  let (<-<): (dispatchable('a), 'a) => unit;
  let (>->): ('a, dispatchable('a)) => unit;
};