type actorPath;
type actorRef = {.
  "parent": actorRef,
  "path": actorPath,
  "name": string
};

type ctx = {. 
    "sender": Js.nullable(actorRef),
    "parent": actorRef,
    "path": actorPath,
    "self": actorRef,
    "name": string,
    "children": Nact_jsMap.t(string, actorRef)
};

type persistentCtx('msgType) = {.
    "sender": Js.nullable(actorRef),
    "parent": actorRef,
    "path": actorPath,
    "self": actorRef,
    "name": string,
    "children": Nact_jsMap.t(string, actorRef),
    "persist": 'msgType => Js.Promise.t(unit),
    "recovering": bool
};

type statefulActor('state, 'msgType) = ('state, 'msgType, ctx) => 'state;
type statelessActor('msgType) = ('msgType, ctx) => unit;
type persistentActor('state, 'msgType) = ('state, 'msgType, persistentCtx('msgType)) => 'state;

[@bs.module "nact"] external spawn : (actorRef,  statefulActor('state, 'msgType), Js.nullable(string)) => actorRef = "spawn";
[@bs.module "nact"] external spawnStateless : (actorRef, statelessActor('msgType), Js.nullable(string)) => actorRef = "spawnStateless";
[@bs.module "nact"] external spawnPersistent : (actorRef, persistentActor('state, 'msgType), string, Js.nullable(string)) => actorRef = "spawnPersistent";

[@bs.module "nact"] external configurePersistence : 'engine => actorRef => unit = "configurePersistence";

[@bs.module "nact"] external stop : (actorRef) => unit = "stop";

[@bs.module "nact"] external start : unit => actorRef = "start";
[@bs.module "nact"] external dispatch : (actorRef, 'msgType, Js.nullable(actorRef)) => unit = "dispatch";
[@bs.module "nact"] external query: (actorRef, 'msgType, int) => Js.Promise.t('expectedResult) = "query";
