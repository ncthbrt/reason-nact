type actorPath = {parts: array<string>, system: string}

type observable

type persistedEvent = {
  data: Js.Json.t,
  sequenceNumber: int,
  key: string,
  createdAt: int,
  tags: array<string>,
}

type persistedSnapshot = {data: Js.Json.t, sequenceNumber: int, key: string, createdAt: int}

type persistenceEngine = {
  events: (string, int, int, array<string>) => observable,
  persist: persistedEvent => Js.Promise.t<unit>,
  takeSnapshot: persistedSnapshot => Js.Promise.t<unit>,
  latestSnapshot: string => Js.Promise.t<persistedSnapshot>,
}

type rec actorRef = {parent: actorRef, path: actorPath, name: string}

type ctx = {
  parent: actorRef,
  path: actorPath,
  self: actorRef,
  name: string,
  children: Js.Dict.t<actorRef>,
  sender: option<actorRef>,
}

type persistentCtx<'msg> = {
  parent: actorRef,
  path: actorPath,
  self: actorRef,
  name: string,
  children: Js.Dict.t<actorRef>,
  persist: 'msg => Js.Promise.t<unit>,
  recovering: option<bool>,
  sender: option<actorRef>,
}

type statefulActor<'state, 'msgType> = ('state, 'msgType, ctx) => Js.Promise.t<'state>

type statelessActor<'msgType> = ('msgType, ctx) => Js.Promise.t<unit>

type persistentActor<'msg, 'state> = ('state, 'msg, persistentCtx<'msg>) => Js.Promise.t<'state>

type persistentQueryFunc<'msg, 'state> = ('state, 'msg) => Js.Promise.t<'state>

type persistentQuery<'state> = unit => Js.Promise.t<'state>

type supervisionAction

type supervisionCtx = {
  parent: actorRef,
  path: actorPath,
  self: actorRef,
  name: string,
  children: Js.Dict.t<actorRef>,
  stop: supervisionAction,
  stopAll: supervisionAction,
  escalate: supervisionAction,
  reset: supervisionAction,
  resetAll: supervisionAction,
  resume: supervisionAction,
  sender: option<actorRef>,
}

type supervisionFunction<'msg, 'parentMsg> = (
  'msg,
  exn,
  supervisionCtx,
) => Js.Promise.t<supervisionAction>

type actorOptions<'msg, 'parentMsg, 'state> = {
  initialStateFunc: option<(. ctx) => 'state>,
  shutdownAfter: option<int>,
  onCrash: option<supervisionFunction<'msg, 'parentMsg>>,
}

type persistentActorOptions<'msg, 'parentMsg, 'state> = {
  initialStateFunc: (. persistentCtx<'msg>) => 'state,
  shutdownAfter: option<int>,
  snapshotEvery: option<int>,
  onCrash: option<supervisionFunction<'msg, 'parentMsg>>,
  decoder: Js.Json.t => 'msg,
  encoder: 'msg => Js.Json.t,
  snapshotEncoder: 'state => Js.Json.t,
  snapshotDecoder: Js.Json.t => 'state,
}

type persistentQueryOptions<'msg, 'state> = {
  initialState: 'state,
  cacheDuration: option<int>,
  snapshotEvery: option<int>,
  decoder: Js.Json.t => 'msg,
  snapshotKey: option<string>,
  encoder: 'msg => Js.Json.t,
  snapshotEncoder: 'state => Js.Json.t,
  snapshotDecoder: Js.Json.t => 'state,
}

@module("nact")
external spawn: (
  actorRef,
  statefulActor<'state, 'msgType>,
  option<string>,
  actorOptions<'msgType, 'parentMsg, 'state>,
) => actorRef = "spawn"

@module("nact")
external spawnStateless: (
  actorRef,
  statelessActor<'msgType>,
  option<string>,
  actorOptions<'msgType, 'parentMsg, unit>,
) => actorRef = "spawnStateless"

type actor

@module("nact/lib/references") @new
external nobody: unit => actorRef = "Nobody"

@module("nact/lib/actor") @val("Actor") external actor: actor = ""

@module("nact")
external spawnPersistent: (
  actorRef,
  persistentActor<'msgType, 'state>,
  string,
  option<string>,
  persistentActorOptions<'msgType, 'parentMsg, 'state>,
) => actorRef = "spawnPersistent"

@module("nact")
external persistentQuery: (
  actorRef,
  persistentQueryFunc<'msgType, 'state>,
  string,
  persistentQueryOptions<'msgType, 'state>,
) => persistentQuery<'state> = "persistentQuery"

type plugin = actorRef => unit

@module("nact")
external configurePersistence: persistenceEngine => plugin = "configurePersistence"

@module("nact") external stop: actorRef => unit = "stop"

@module("nact") @variadic
external start: array<plugin> => actorRef = "start"

@module("nact") external dispatch: (actorRef, 'msgType) => unit = "dispatch"

@module("nact")
external dispatchWithSender: (actorRef, 'msgType, option<actorRef>) => unit = "dispatch"

@module("nact")
external query: (actorRef, actorRef => 'msgType, int) => Js.Promise.t<'expectedResult> = "query"
