module StringSet = Nact_stringSet;

type persistenceEngine = Nact_bindings.persistenceEngine;

type actorPath =
  | ActorPath(Nact_bindings.actorPath);

type actorRef('msg) =
  | ActorRef(Nact_bindings.actorRef);

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

let mapCtx = (untypedCtx: Nact_bindings.ctx) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  path: ActorPath(untypedCtx##path),
  children: untypedCtx##children |> Nact_jsMap.keys |> StringSet.fromJsArray
};

let mapPersist = (persist, msg) => persist(msg);

let mapPersistentCtx = (untypedCtx: Nact_bindings.persistentCtx('incoming)) => {
  name: untypedCtx##name,
  self: ActorRef(untypedCtx##self),
  parent: ActorRef(untypedCtx##parent),
  path: ActorPath(untypedCtx##path),
  recovering: untypedCtx##recovering,
  persist: mapPersist(untypedCtx##persist),
  children: untypedCtx##children |> Nact_jsMap.keys |> StringSet.fromJsArray
};

type statefulActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, ctx('msg, 'parentMsg)) => Js.Promise.t('state);

type statelessActor('msg, 'parentMsg) = ('msg, ctx('msg, 'parentMsg)) => Js.Promise.t(unit);

type persistentActor('state, 'msg, 'parentMsg) =
  ('state, 'msg, persistentCtx('msg, 'parentMsg)) => Js.Promise.t('state);

type timeout = {duration: int};

type interval = {
  duration: int,
  messages: int
};

let defaultOrValue = (opt, defaultValue) =>
  switch opt {
  | Some(value) => value
  | None => defaultValue
  };

let after = (~hours=?, ~minutes=?, ~seconds=?, ~milliseconds=?, ()) => {
  duration:
    defaultOrValue(hours, 0)
    + defaultOrValue(minutes, 0)
    + defaultOrValue(seconds, 0)
    + defaultOrValue(milliseconds, 0)
};

let every = (~messages=?, ~hours=?, ~minutes=?, ~seconds=?, ~milliseconds=?, ()) => {
  duration:
    defaultOrValue(hours, 0)
    + defaultOrValue(minutes, 0)
    + defaultOrValue(seconds, 0)
    + defaultOrValue(milliseconds, 0),
  messages: defaultOrValue(messages, 0)
};

let spawn:
  (
    ~name: string=?,
    ~timeout: timeout=?,
    actorRef('parentMsg),
    statefulActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg) =
  (~name=?, ~timeout=?, ActorRef(parent), func, initialState) => {
    let duration =
      switch timeout {
      | Some({duration}) => Js.Nullable.return({"duration": duration})
      | None => Js.Nullable.undefined
      };
    let options: Nact_bindings.actorOptions = {"timeout": duration};
    let f = (possibleState, msg, ctx) => {
      let state =
        switch (Js.Nullable.to_opt(possibleState)) {
        | None => initialState
        | Some(concreteState) => concreteState
        };
      func(state, msg, mapCtx(ctx))
    };
    let untypedRef =
      switch name {
      | Some(concreteName) =>
        Nact_bindings.spawn(parent, f, Js.Nullable.return(concreteName), options)
      | None => Nact_bindings.spawn(parent, f, Js.Nullable.undefined, options)
      };
    ActorRef(untypedRef)
  };

let spawnStateless:
  (~name: string=?, ~timeout: timeout=?, actorRef('parentMsg), statelessActor('msg, 'parentMsg)) =>
  actorRef('msg) =
  (~name=?, ~timeout=?, ActorRef(parent), func) => {
    let timeout =
      switch timeout {
      | Some({duration}) => Js.Nullable.return({"duration": duration})
      | None => Js.Nullable.undefined
      };
    let options: Nact_bindings.actorOptions = {"timeout": timeout};
    let f = (msg, ctx) => func(msg, mapCtx(ctx));
    let untypedRef =
      switch name {
      | Some(concreteName) =>
        Nact_bindings.spawnStateless(parent, f, Js.Nullable.return(concreteName), options)
      | None => Nact_bindings.spawnStateless(parent, f, Js.Nullable.undefined, options)
      };
    ActorRef(untypedRef)
  };

let spawnPersistent:
  (
    ~key: string,
    ~name: string=?,
    ~timeout: timeout=?,
    ~snapshot: interval=?,
    actorRef('parentMsg),
    persistentActor('state, 'msg, 'parentMsg),
    'state
  ) =>
  actorRef('msg) =
  (~key, ~name=?, ~timeout=?, ~snapshot=?, ActorRef(parent), func, initialState) => {
    let timeout =
      switch timeout {
      | Some({duration}) => Js.Nullable.return({"duration": duration})
      | None => Js.Nullable.undefined
      };
    let snapshot =
      switch snapshot {
      | Some({duration: 0, messages: 0}) => Js.Nullable.undefined
      | Some({duration, messages: 0}) =>
        Js.Nullable.return({
          "duration": Js.Nullable.return(duration),
          "messages": Js.Nullable.undefined
        })
      | Some({duration, messages}) =>
        Js.Nullable.return({
          "duration": Js.Nullable.return(duration),
          "messages": Js.Nullable.return(messages)
        })
      | None => Js.Nullable.undefined
      };
    let options: Nact_bindings.persistentActorOptions = {"timeout": timeout, "snapshot": snapshot};
    let f = (possibleState, msg, ctx) => {
      let state =
        switch (Js.Nullable.to_opt(possibleState)) {
        | None => initialState
        | Some(concreteState) => concreteState
        };
      func(state, msg, mapPersistentCtx(ctx))
    };
    let untypedRef =
      switch name {
      | Some(concreteName) =>
        Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.return(concreteName), options)
      | None => Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.undefined, options)
      };
    ActorRef(untypedRef)
  };

let stop = (ActorRef(reference)) => Nact_bindings.stop(reference);

let start = (~persistenceEngine=?, ()) => {
  let untypedRef =
    switch persistenceEngine {
    | Some(engine) => Nact_bindings.start([|Nact_bindings.configurePersistence(engine)|])
    | None => Nact_bindings.start([||])
    };
  ActorRef(untypedRef)
};

let dispatch = (ActorRef(recipient), msg) => Nact_bindings.dispatch(recipient, msg);

exception QueryTimeout(timeout);

let query = (~timeout: timeout, ActorRef(recipient), msgF) => {
  let f = (tempReference) => msgF(ActorRef(tempReference));
  Nact_bindings.query(recipient, f, timeout.duration)
  |> Js.Promise.catch((_) => Js.Promise.reject(QueryTimeout(timeout)))
};