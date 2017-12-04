module StringSet = Nact_stringSet;

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

let spawn:
  (~name: string=?, actorRef('parentMsg), statefulActor('state, 'msg, 'parentMsg), 'state) =>
  actorRef('msg) =
  (~name=?, ActorRef(parent), func, initialState) => {
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
      | Some(concreteName) => Nact_bindings.spawn(parent, f, Js.Nullable.return(concreteName))
      | None => Nact_bindings.spawn(parent, f, Js.Nullable.undefined)
      };
    ActorRef(untypedRef)
  };

let spawnStateless = (~name=?, ActorRef(parent), func) => {
  let f = (msg, ctx) => func(msg, mapCtx(ctx));
  let untypedRef =
    switch name {
    | Some(concreteName) =>
      Nact_bindings.spawnStateless(parent, f, Js.Nullable.return(concreteName))
    | None => Nact_bindings.spawnStateless(parent, f, Js.Nullable.undefined)
    };
  ActorRef(untypedRef)
};

let spawnPersistent = (~key, ~name=?, ActorRef(parent), func, initialState) => {
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
      Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.return(concreteName))
    | None => Nact_bindings.spawnPersistent(parent, f, key, Js.Nullable.undefined)
    };
  ActorRef(untypedRef)
};

let stop = (ActorRef(reference)) => Nact_bindings.stop(reference);

let start = () => {
  let untypedRef = Nact_bindings.start();
  ActorRef(untypedRef)
};

let dispatch = (ActorRef(recipient), msg) => Nact_bindings.dispatch(recipient, msg);

exception QueryTimeout(int);

exception ActorNotAvailable;

let query = (~timeout, ActorRef(recipient), msgF) =>
  Nact_bindings.query(recipient, msgF(ActorRef(recipient)), timeout)
  |> Js.Promise.catch((_) => Js.Promise.reject(QueryTimeout(timeout)));