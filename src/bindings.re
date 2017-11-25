type actorPath;
type actorRef = {.
  "parent": actorRef,
  "path": actorPath,
  "name": string
};

module JsMap {
    type map('key,'value);
  	[@bs.new] external create: array(('key, 'value)) => map('key, 'value) = "Map";
    [@bs.send] external find: map('key,'value) => 'key => 'value = "get";
    [@bs.send] external has: map('key,'value) => 'key => bool = "has";
    [@bs.send] external get: map('key,'value) => 'key => 'value = "get";
    [@bs.send] external keys: map('key,'value) => array('key) = "keys";
    [@bs.send] external values: map('key,'value) => array('value) = "values";
    [@bs.send] external set: map('key,'value) => 'key => 'value => map('key,'value) = "set";
    [@bs.send] external entries: map('key,'value) => array(('key, 'value)) = "entries";
    [@bs.send] external clear: map('key, 'value) => unit = "clear";
    [@bs.send] external delete: map('key,'value) => 'key => bool = "delete";
    [@bs.send] external forEach: map('key,'value) => ('value => 'key => map('key, 'value) => unit) => unit = "forEach";
    [@bs.get] external size: map('key, 'value) => int = "size";
};

type ctx = {. 
    "sender": actorRef,
    "parent": actorRef,
    "path": actorPath,
    "self": actorRef,
    "name": string,
    "children": JsMap.map(string, actorRef)
};

type persistentCtx('msgType) = {.
    "sender": actorRef,
    "parent": actorRef,
    "path": actorPath,
    "self": actorRef,
    "name": string,
    "children": JsMap.map(string, actorRef),
    "persist": 'msgType => Js.Promise.t(unit),
    "recovering": bool
};

type statefulActor('state, 'msgType) = ('state, 'msgType, ctx) => 'state;
type statelessActor('msgType) = ('msgType, ctx) => unit;
type persistentActor('state, 'msgType) = ('state, 'msgType, persistentCtx('msgType)) => 'state;

[@bs.module "nact"] external spawn : (actorRef,  statefulActor('state, 'msgType), string) => actorRef = "spawn";
[@bs.module "nact"] external spawnStateless : (actorRef, statelessActor('msgType), string) => actorRef = "spawnStateless";
[@bs.module "nact"] external spawnPersistent : (actorRef, persistentActor('state, 'msgType), string, string) => actorRef = "spawnPersistent";

[@bs.module "nact"] external configurePersistence : 'engine => actorRef => unit = "configurePersistence";

[@bs.module "nact"] external stop : (actorRef) => unit = "stop";

[@bs.module "nact"] external start : ('options) => actorRef = "start";
[@bs.module "nact"] external dispatch : (actorRef, 'msgType, 'sender) => unit = "dispatch";
[@bs.module "nact"] external query: (actorRef, 'msgType, int) => Js.Promise.t('expectedResult) = "query";
