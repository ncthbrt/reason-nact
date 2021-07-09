open Js.Promise

open Js.Nullable

type persistenceEngine = Nact_bindings.persistenceEngine

type untypedRef = Nact_bindings.actorRef

type actorRef<'msg> = ActorRef(Nact_bindings.actorRef)

module Interop = {
  let fromUntypedRef = reference => ActorRef(reference)
  let toUntypedRef = (ActorRef(reference)) => reference
  let dispatch = Nact_bindings.dispatch
  let dispatchWithSender = Nact_bindings.dispatchWithSender
}

type actorPath = ActorPath(Nact_bindings.actorPath)

module ActorPath = {
  let fromReference = (ActorRef(actor)) => ActorPath(actor["path"])
  let systemName = (ActorPath(path)) => path["system"]
  let toString = (ActorPath(path)) =>
    "system:" ++
    (path["system"] ++
    ("//" ++ String.concat("/", Belt.List.fromArray(path["parts"]))))
  let parts = (ActorPath(path)) => Belt.List.fromArray(path["parts"])
}

type systemMsg

%%raw(`
/* This code is to handle how bucklescript sometimes represents variants */

var WrappedVariant = '_wvariant';
var WrappedEvent = '_wevent';
function unsafeEncoder(obj) {
  var data = JSON.stringify(obj, function (key, value) {
    if (value && Array.isArray(value) && value.tag !== undefined) {
      var r = {};
      r.values = value.slice();
      r.tag = value.tag;
      r.type = WrappedVariant;
      return r;
    } else {
      return value;
    }
  });
  return { data: JSON.parse(data), type: WrappedEvent };
};

function unsafeDecoder(result) {
  if(result && typeof(result) === 'object' && result.type === WrappedEvent) {
    var serialized = result.serialized || JSON.stringify(result.data);
    return JSON.parse(serialized, (key, value) => {
      if (value && typeof (value) === 'object' && value.type === WrappedVariant) {
        var values = value.values;
        values.tag = value.tag;
        return values;
      } else {
        return value;
      }
    });
  } else {
    return result;
  }
};
`)

type decoder<'a> = Js.Json.t => 'a

type encoder<'a> = 'a => Js.Json.t

@val external unsafeDecoder: Js.Json.t => 'msg = "unsafeDecoder"

@val external unsafeEncoder: 'msg => Js.Json.t = "unsafeEncoder"

type ctx<'msg, 'parentMsg> = {
  parent: actorRef<'parentMsg>,
  path: actorPath,
  self: actorRef<'msg>,
  children: Belt.Set.String.t,
  name: string,
  /* Sender added for interop purposes. Not to be used for reason only code */
  sender: Js.nullable<untypedRef>,
}

type persistentCtx<'msg, 'parentMsg> = {
  parent: actorRef<'parentMsg>,
  path: actorPath,
  self: actorRef<'msg>,
  name: string,
  persist: 'msg => Js.Promise.t<unit>,
  children: Belt.Set.String.t,
  recovering: bool,
  /* Sender added for interop purposes. Not to be used for reason only code */
  sender: Js.nullable<untypedRef>,
}

let mapCtx = (untypedCtx: Nact_bindings.ctx) => {
  name: untypedCtx["name"],
  self: ActorRef(untypedCtx["self"]),
  parent: ActorRef(untypedCtx["parent"]),
  path: ActorPath(untypedCtx["path"]),
  children: untypedCtx["children"] |> Nact_jsMap.keys |> Belt.Set.String.fromArray,
  sender: untypedCtx["sender"],
}

let mapPersistentCtx = (untypedCtx: Nact_bindings.persistentCtx<'incoming>) => {
  name: untypedCtx["name"],
  self: ActorRef(untypedCtx["self"]),
  parent: ActorRef(untypedCtx["parent"]),
  path: ActorPath(untypedCtx["path"]),
  recovering: untypedCtx["recovering"]->Js.Nullable.toOption->Belt.Option.getWithDefault(false),
  persist: untypedCtx["persist"],
  children: untypedCtx["children"] |> Nact_jsMap.keys |> Belt.Set.String.fromArray,
  sender: untypedCtx["sender"],
}

type supervisionCtx<'msg, 'parentMsg> = {
  parent: actorRef<'parentMsg>,
  path: actorPath,
  self: actorRef<'msg>,
  name: string,
  children: Belt.Set.String.t,
  sender: Js.nullable<untypedRef>,
}

let mapSupervisionCtx = (untypedCtx: Nact_bindings.supervisionCtx) => {
  name: untypedCtx["name"],
  self: ActorRef(untypedCtx["self"]),
  parent: ActorRef(untypedCtx["parent"]),
  path: ActorPath(untypedCtx["path"]),
  children: untypedCtx["children"] |> Nact_jsMap.keys |> Belt.Set.String.fromArray,
  sender: untypedCtx["sender"],
}

type supervisionAction =
  | Stop
  | StopAll
  | Reset
  | ResetAll
  | Escalate
  | Resume

type supervisionPolicy<'msg, 'parentMsg> = (
  'msg,
  exn,
  supervisionCtx<'msg, 'parentMsg>,
) => Js.Promise.t<supervisionAction>

type statefulSupervisionPolicy<'msg, 'parentMsg, 'state> = (
  'msg,
  exn,
  'state,
  supervisionCtx<'msg, 'parentMsg>,
) => ('state, Js.Promise.t<supervisionAction>)

let mapSupervisionFunction = optionalF =>
  switch optionalF {
  | None => Js.Nullable.undefined
  | Some(f) =>
    Js.Nullable.return((msg, err, ctx) =>
      f(msg, err, mapSupervisionCtx(ctx)) |> then_(decision =>
        resolve(
          switch decision {
          | Stop => ctx["stop"]
          | StopAll => ctx["stopAll"]
          | Reset => ctx["reset"]
          | ResetAll => ctx["resetAll"]
          | Escalate => ctx["escalate"]
          | Resume => ctx["resume"]
          },
        )
      )
    )
  }

type statefulActor<'state, 'msg, 'parentMsg> = (
  'state,
  'msg,
  ctx<'msg, 'parentMsg>,
) => Js.Promise.t<'state>

type statelessActor<'msg, 'parentMsg> = ('msg, ctx<'msg, 'parentMsg>) => Js.Promise.t<unit>

type persistentActor<'state, 'msg, 'parentMsg> = (
  'state,
  'msg,
  persistentCtx<'msg, 'parentMsg>,
) => Js.Promise.t<'state>

type persistentQuery<'state> = unit => Js.Promise.t<'state>

let useStatefulSupervisionPolicy = (f, initialState) => {
  let state = ref(initialState)
  (msg, err, ctx) => {
    let (nextState, promise) = f(msg, err, state.contents, ctx)
    state := nextState
    promise
  }
}

let spawn = (~name=?, ~shutdownAfter=?, ~onCrash=?, ActorRef(parent), func, initialState) => {
  let options = {
    "initialStateFunc": Js.Nullable.return((. ctx) => initialState(mapCtx(ctx))),
    "shutdownAfter": fromOption(shutdownAfter),
    "onCrash": mapSupervisionFunction(onCrash),
  }
  let f = (state, msg: 'msg, ctx) =>
    try func(state, msg, mapCtx(ctx)) catch {
    | err => reject(err)
    }
  let untypedRef = Nact_bindings.spawn(parent, f, fromOption(name), options)
  ActorRef(untypedRef)
}

let spawnStateless = (~name=?, ~shutdownAfter=?, ActorRef(parent), func) => {
  let options = {
    "shutdownAfter": fromOption(shutdownAfter),
    "initialStateFunc": Js.Nullable.undefined,
    "onCrash": mapSupervisionFunction(None),
  }
  let f = (msg, ctx) =>
    try func(msg, mapCtx(ctx)) catch {
    | e => reject(e)
    }
  let untypedRef = Nact_bindings.spawnStateless(parent, f, fromOption(name), options)
  ActorRef(untypedRef)
}

let spawnPersistent = (
  ~key,
  ~name=?,
  ~shutdownAfter=?,
  ~snapshotEvery=?,
  ~onCrash: option<supervisionPolicy<'msg, 'parentMsg>>=?,
  ~decoder: option<decoder<'msg>>=?,
  ~stateDecoder: option<decoder<'state>>=?,
  ~encoder: option<encoder<'msg>>=?,
  ~stateEncoder: option<encoder<'state>>=?,
  ActorRef(parent),
  func,
  initialState: persistentCtx<'msg, 'parentMsg> => 'state,
) => {
  let decoder = decoder->Belt.Option.getWithDefault(unsafeDecoder)
  let stateDecoder = stateDecoder->Belt.Option.getWithDefault(unsafeDecoder)
  let stateEncoder = stateEncoder->Belt.Option.getWithDefault(unsafeEncoder)
  let encoder = encoder->Belt.Option.getWithDefault(unsafeEncoder)
  let options: Nact_bindings.persistentActorOptions<'msg, 'parentMsg, 'state> = {
    "initialStateFunc": (. ctx) => initialState(mapPersistentCtx(ctx)),
    "shutdownAfter": fromOption(shutdownAfter),
    "onCrash": mapSupervisionFunction(onCrash),
    "snapshotEvery": fromOption(snapshotEvery),
    "encoder": encoder,
    "decoder": decoder,
    "snapshotEncoder": stateEncoder,
    "snapshotDecoder": stateDecoder,
  }
  let f = (state, msg, ctx) =>
    try func(state, msg, mapPersistentCtx(ctx)) catch {
    | err => reject(err)
    }
  let untypedRef = Nact_bindings.spawnPersistent(parent, f, key, fromOption(name), options)
  ActorRef(untypedRef)
}

let persistentQuery = (
  ~key,
  ~snapshotKey=?,
  ~cacheDuration=?,
  ~snapshotEvery=?,
  ~decoder=?,
  ~stateDecoder=?,
  ~encoder=?,
  ~stateEncoder=?,
  ActorRef(actor),
  func,
  initialState,
) => {
  let decoder = decoder->Belt.Option.getWithDefault(unsafeDecoder)
  let stateDecoder = stateDecoder->Belt.Option.getWithDefault(unsafeDecoder)
  let stateEncoder = stateEncoder->Belt.Option.getWithDefault(unsafeEncoder)
  let encoder = encoder->Belt.Option.getWithDefault(unsafeEncoder)
  let options: Nact_bindings.persistentQueryOptions<'msg, 'state> = {
    "initialState": initialState,
    "cacheDuration": fromOption(cacheDuration),
    "snapshotEvery": fromOption(snapshotEvery),
    "snapshotKey": fromOption(snapshotKey),
    "encoder": encoder,
    "decoder": decoder,
    "snapshotEncoder": stateEncoder,
    "snapshotDecoder": stateDecoder,
  }
  let f = (state, msg) =>
    try func(state, msg) catch {
    | err => reject(err)
    }
  Nact_bindings.persistentQuery(actor, f, key, options)
}

let stop = (ActorRef(reference)) => Nact_bindings.stop(reference)

let dispatch = (ActorRef(recipient), msg) => Nact_bindings.dispatch(recipient, msg)

let nobody = () => ActorRef(Nact_bindings.nobody())

let spawnAdapter = (~name=?, parent, mapping) => {
  let f = (msg, _) => resolve(dispatch(parent, mapping(msg)))
  switch name {
  | Some(name) => spawnStateless(~name, parent, f)
  | None => spawnStateless(parent, f)
  }
}

let start = (~name: option<string>=?, ~persistenceEngine=?, ()) => {
  let plugins = switch persistenceEngine {
  | Some(engine) => list{Nact_bindings.configurePersistence(engine)}
  | None => list{}
  }
  let plugins = switch name {
  | Some(name) => list{Obj.magic({"name": name}), ...plugins}
  | None => plugins
  }
  switch plugins {
  | list{a, b, ..._} => ActorRef(Nact_bindings.start([a, b]))
  | list{a} => ActorRef(Nact_bindings.start([a]))
  | list{} => ActorRef(Nact_bindings.start([]))
  }
}

exception QueryTimeout(int)

let query = (~timeout: int, ActorRef(recipient), msgF) => {
  let f = tempReference => msgF(ActorRef(tempReference))
  Nact_bindings.query(recipient, f, timeout) |> catch(_ => reject(QueryTimeout(timeout)))
}

let milliseconds = 1

let millisecond = milliseconds

let seconds = 1000 * milliseconds

let second = seconds

let minutes = 60 * seconds

let minute = minutes

let hours = 60 * minutes

let messages = 1

let message = 1

module Operators = {
  let \"<-<" = (actorRef, msg) => dispatch(actorRef, msg)
  let \">->" = (msg, actorRef) => dispatch(actorRef, msg)
  let \"<?" = (actor, (f, timeout)) => query(~timeout, actor, f)
}
