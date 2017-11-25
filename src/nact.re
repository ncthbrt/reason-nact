type untypedActorRef;
type actorPath;
type actorRef('incoming);

type ctx('incoming, 'outgoing, 'parentIncoming) = {
  sender: option(actorRef('outgoing)),
  parent: actorRef('parentIncoming),
  path: actorPath,
  self: actorRef('incoming),
  name: string,
  children: Immutable.Map.t(string, string)
};

type persistentCtx('incoming, 'outgoing, 'parentIncoming) = {
    sender: option(actorRef('outgoing)),
    parent: actorRef('parentIncoming),
    path: actorPath,
    self: actorRef('incoming),
    name: string,
    children: Immutable.Map.t(string, string),
    persist: 'incoming => Js.Promise.t(unit),
    recovering: bool    
};

type statefulActor('state, 'incoming, 'outgoing, 'parentIncoming) = 
    ('state, 'incoming, ctx('incoming, 'outgoing, 'parentIncoming)) => 'state;

type statelessActor('incoming, 'outgoing, 'parentIncoming) = 
  ('incoming, ctx('incoming, 'outgoing, 'parentIncoming)) => unit;

type persistentActor('state, 'incoming, 'outgoing, 'parentIncoming) = 
    ('state, 'incoming, persistentCtx('incoming, 'outgoing, 'parentIncoming)) => 'state;

module type Nact = {
    let spawn : (actorRef('parentIncoming),  statefulActor('state, 'incoming, 'outgoing, 'parentIncoming), string) => actorRef('incoming);
    let spawnStateless : (actorRef('parentIncoming), statelessActor('incoming, 'outgoing, 'parentIncoming), string) => actorRef('incoming);
    let spawnPersistent : (actorRef('parentIncoming), persistentActor('state, 'incoming, 'outgoing, 'parentIncoming), string) => actorRef('incoming);
    let stop : (actorRef('msgType)) => unit;    
    let start : ('options) => actorRef(unit);
    let dispatch : (actorRef('msgType), 'msgType, actorRef('sender)) => unit;
    let query: (actorRef('msgType), 'msgType, int) => Js.Promise.t('responseMsg);
};
