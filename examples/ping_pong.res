open Nact

let system = start()

type rec msgType = Msg(actorRef<msgType>, string)

let ping: actorRef<msgType> = spawnStateless(~name="ping", system, (Msg(sender, msg), ctx) => {
  Js.log(msg)
  sender->dispatch(Msg(ctx.self, ctx.name))->Js.Promise.resolve
})

let pong: actorRef<msgType> = spawnStateless(~name="pong", system, (Msg(sender, msg), ctx) => {
  Js.log(msg)
  sender->dispatch(Msg(ctx.self, ctx.name))->Js.Promise.resolve
})

ping->dispatch(Msg(pong, "hello"))

Js.Global.setTimeout(() => stop(system), 100)->ignore
