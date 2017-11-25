type untypedActorRef;

type actorPath;
type actorRef('msgType);
    
type ctx('msgType, 'responseType, 'parentMsgType) = {
  sender: actorRef('responseType),
  parent: actorRef('parentMsgType),
  path: actorPath,
  self: actorRef('msgType),
  name: string,
  children: Immutable.Map.t(string, string)
};

type persistentCtx('msgType, 'responseType, 'parentMsgType) = {
    sender: actorRef('responseType),
    parent: actorRef('parentMsgType),
    path: actorPath,
    self: actorRef('msgType),
    name: string,
    children: Immutable.Map.t(string, string),
    persist: 'msgType => Js.Promise.t(unit),
    recovering: bool    
};

type statefulActor('state, 'msg, 'responseMsg, 'parentMsg) = 
    ('state, 'msg, ctx('msg, 'responseMsg, 'parentMsg)) => 'state;

type statelessActor('msg, 'responseMsg, 'parentMsg) = 
  ('msg, ctx('msg, 'responseMsg, 'parentMsg)) => unit;
  
type persistentActor('state, 'msg, 'responseMsg, 'parentMsg) = 
    ('state, 'msg, persistentCtx('msg, 'responseMsg, 'parentMsg)) => 'state;


module type Nact = {
    let spawn : (actorRef('parentMsg),  statefulActor('state, 'msgType, 'responseMsg, 'parentMsg), string) => actorRef('msgType);
    let spawnStateless : (actorRef('parentMsg), statelessActor('msgType, 'responseMsg, 'parentMsg), string) => actorRef('msgType);
    let spawnPersistent : (actorRef('parentMsg), persistentActor('state, 'msgType, 'responseMsg, 'parentMsg), string) => actorRef('msgType);
    let stop : (actorRef('msgType)) => unit;    
    let start : ('options) => actorRef(unit);
    let dispatch : (actorRef('msgType), 'msgType, actorRef('sender)) => unit;
    let query: (actorRef('msgType), 'msgType, int) => Js.Promise.t('responseMsg);
};
