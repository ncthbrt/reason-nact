module StringMap = Nact_stringMap;

type untypedActorRef = 
  | UntypedActorRef(Nact_bindings.actorRef);
  
type untypedActorMap = StringMap.t(untypedActorRef);


type actorPath =
  | ActorPath(Nact_bindings.actorPath);

type actorRef('incoming, 'outgoing) =
  | ActorRef(Nact_bindings.actorRef);


type ctx('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing) = {
  sender: option(actorRef('outgoing, 'senderOutgoing)),
  parent: actorRef('parentIncoming, 'parentOutgoing),
  path: actorPath,
  self: actorRef('incoming, 'outgoing),
  children: untypedActorMap,
  name: string
};

type persistentCtx('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing) = {
  sender: option(actorRef('outgoing, 'senderOutgoing)),
  parent: actorRef('parentIncoming, 'parentOutgoing),
  path: actorPath,
  self: actorRef('incoming, 'outgoing),
  name: string,  
  persist: 'incoming => Js.Promise.t(unit),
  children: untypedActorMap,
  recovering: bool    
};

let mapSender = (sender) =>
  switch (Js.Nullable.to_opt(sender)) {
  | Some(sndr) => Some(ActorRef(sndr))
  | None => None
  };

let createUntypedRef = x => UntypedActorRef(x);
let mapCtx = (untypedCtx: Nact_bindings.ctx) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  sender: mapSender(untypedCtx##sender),
  path: ActorPath(untypedCtx##path),
  children: untypedCtx##children 
    |> Nact_jsMap.mapValues(createUntypedRef)
    |> StringMap.fromJsMap
};

let mapPersist = (persist, msg) => persist(msg);

let mapPersistentCtx = (untypedCtx: Nact_bindings.persistentCtx('incoming)) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  sender: mapSender(untypedCtx##sender),
  path: ActorPath(untypedCtx##path),
  recovering: untypedCtx##recovering,
  persist: mapPersist(untypedCtx##persist),
  children: untypedCtx##children 
            |> Nact_jsMap.mapValues(createUntypedRef) 
            |> StringMap.fromJsMap
};


type statefulActor('state, 'incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing) =
  (
    'state,
    'incoming,
    ctx('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)
  ) =>
  'state;

type statelessActor('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing) =
  ('incoming, ctx('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)) => unit;

type persistentActor(
  'state,
  'incoming,
  'outgoing,
  'parentIncoming,
  'parentOutgoing,
  'senderOutgoing
) =
  (
    'state,
    'incoming,
    persistentCtx('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)
  ) =>
  'state;

let spawn:
  (
    ~name: string=?,
    actorRef('parentIncoming, 'parentOutgoing),
    statefulActor('state, 'incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)
  ) =>
  actorRef('incoming, 'outgoing) =
  (~name=?, ActorRef(parent), func) => {
    let f = (state, msg, ctx) => func(state, msg, mapCtx(ctx));
    let untypedRef =
      switch name {
      | Some(concreteName) => Nact_bindings.spawn(parent, f, Js.Nullable.return(concreteName))
      | None => Nact_bindings.spawn(parent, f, Js.Nullable.undefined)
      };
    ActorRef(untypedRef)
  };

let spawnStateless:
  (
    ~name: string=?,
    actorRef('parentIncoming, 'parentOutgoing),
    statelessActor('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)
  ) =>
  actorRef('incoming, 'outgoing) =
  (~name=?, ActorRef(parent), func) => {
    let f = (msg, ctx) => func(msg, mapCtx(ctx));
    let untypedRef =
      switch name {
      | Some(concreteName) => Nact_bindings.spawnStateless(parent, f, Js.Nullable.return(concreteName))
      | None => Nact_bindings.spawnStateless(parent, f, Js.Nullable.undefined)
      };
    ActorRef(untypedRef)
  };

let spawnPersistent:
  (
    ~key: string,
    ~name: string=?,
    actorRef('parentIncoming, 'parentOutgoing),
    persistentActor(
      'state,
      'incoming,
      'outgoing,
      'parentIncoming,
      'parentOutgoing,
      'senderOutgoing
    )
  ) =>
  actorRef('incoming, 'outgoing) =
  (~key, ~name=?, ActorRef(parent), func) => {
    let f = (state, msg, ctx) => func(state, msg, mapPersistentCtx(ctx));
    let untypedRef =
      switch name {
      | Some(concreteName) =>
      Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.return(concreteName))
      | None => Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.undefined)
      };
    ActorRef(untypedRef)
  };

let stop = (ActorRef(reference)) => Nact_bindings.stop(reference);

let start = () => {
  let untypedRef = Nact_bindings.start();
  ActorRef(untypedRef);
};

let dispatch = (~sender=?, ActorRef(actor), msg) =>
  switch sender {
  | Some(ActorRef(concreteSender)) =>
     Nact_bindings.dispatch(actor, msg, Js.Nullable.return(concreteSender))
  | None => Nact_bindings.dispatch(actor, msg, Js.Nullable.undefined)
  };

exception QueryTimeout(int);

let query: (~timeout:int, actorRef('incoming, 'outgoing), 'incoming) => Js.Promise.t('outgoing) =
  (~timeout, ActorRef(actor), msg) =>
    Nact_bindings.query(actor, msg, timeout)
    |> Js.Promise.catch((_) => Js.Promise.reject(QueryTimeout(timeout)));
