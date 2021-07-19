open Nact

let system = Nact.start()

type rec world = World(actorRef<world>)

type hello = Hello(actorRef<world>)

let world: actorRef<world> = spawnStateless(~name="world", system, (World(sender), ctx) => {
  Js.log("world!!")
  sender->dispatch(World(ctx.self))->Js.Promise.resolve
})

let createAdapterIfNotExists = (parent, adapterOpt) =>
  switch adapterOpt {
  | Some(adapter) => adapter
  | None => spawnAdapter(parent, (World(sender)) => Hello(sender))
  }

let hello = spawn(
  ~name="hello",
  system,
  (adapterOpt, Hello(sender), ctx) => {
    let adapter = createAdapterIfNotExists(ctx.self, adapterOpt)
    Js.log("Hello ")
    sender->dispatch(World(adapter))
    Js.Promise.resolve(Some(adapter))
  },
  _ => None,
)

hello->dispatch(Hello(world))
