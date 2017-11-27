module StringMap = Nact_stringMap;

type actorPath;

type untypedActorRef;

type untypedActorMap = StringMap.t(untypedActorRef);

type actorRef('incoming, 'outgoing);

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
  actorRef('incoming, 'outgoing);

let spawnStateless:
  (
    ~name: string=?,
    actorRef('parentIncoming, 'parentOutgoing),
    statelessActor('incoming, 'outgoing, 'parentIncoming, 'parentOutgoing, 'senderOutgoing)
  ) =>
  actorRef('incoming, 'outgoing);

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
  actorRef('incoming, 'outgoing);

let stop: actorRef('incoming, 'outgoing) => unit;

let start: unit => actorRef(unit, unit);

let dispatch:
  (
    ~sender: actorRef('senderIncoming, 'senderOutgoing)=?,
    actorRef('incoming, 'outgoing),
    'incoming
  ) =>
  unit;

exception QueryTimeout(int);

let query: (~timeout: int, actorRef('incoming, 'outgoing), 'incoming) => Js.Promise.t('outgoing);