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

type statefulActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, ctx('msg, 'parentMsg)) => Js.Promise.t('state);

type statelessActor('msg, 'parentMsg) = ('msg, ctx('msg, 'parentMsg)) => Js.Promise.t(unit);

type persistentActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state);

let spawn:
  (~name: string=?, actorRef('parentMsg), statefulActor('state, 'msg, 'parentMsg), 'state) =>
  actorRef('msg);

let spawnStateless:
  (~name: string=?, actorRef('parentMsg), statelessActor('msg, 'parentMsg)) => actorRef('msg);

let spawnPersistent:
  (
    ~key: string,
    ~name: string=?,
    actorRef('parentMsg),
    persistentActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg);

let stop: actorRef('msg) => unit;

let start: (~persistenceEngine: persistenceEngine=?, unit) => actorRef(unit);

let dispatch: (actorRef('msg), 'msg) => unit;

exception ActorNotAvailable;

exception QueryTimeout(int);

let query: (~timeout: int, actorRef('msg), actorRef('outgoing) => 'msg) => Js.Promise.t('outgoing);