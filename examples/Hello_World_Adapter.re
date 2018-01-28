open Nact;

open Nact.Operators;

let system = Nact.start();

type world =
  | World(actorRef(world));

type hello =
  | Hello(actorRef(world));

let world: actorRef(world) =
  spawnStateless(
    ~name="world",
    system,
    (World(sender), ctx) => {
      print_endline("world!!");
      sender <-< World(ctx.self) |> Js.Promise.resolve
    }
  );

let createAdapterIfNotExists = (parent, adapterOpt) =>
  switch adapterOpt {
  | Some(adapter) => adapter
  | None => spawnAdapter(parent, (World(sender)) => Hello(sender))
  };

let hello =
  spawn(
    ~name="hello",
    system,
    (adapterOpt, Hello(sender), ctx) => {
      let adapter = createAdapterIfNotExists(ctx.self, adapterOpt);
      print_string("Hello ");
      sender <-< World(adapter);
      Js.Promise.resolve(Some(adapter))
    },
    None
  );

hello <-< Hello(world);