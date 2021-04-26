open Nact;

open Nact.Operators;

let system = start();

type msgType =
  | Msg(actorRef(msgType), string);

let ping: actorRef(msgType) =
  spawnStateless(
    ~name="ping",
    system,
    (Msg(sender, msg), ctx) => {
      print_endline(msg);
      sender <-< Msg(ctx.self, ctx.name) |> Js.Promise.resolve
    }
  );

let pong: actorRef(msgType) =
  spawnStateless(
    ~name="pong",
    system,
    (Msg(sender, msg), ctx) => {
      print_endline(msg);
      sender <-< Msg(ctx.self, ctx.name) |> Js.Promise.resolve
    }
  );

ping <-< Msg(pong, "hello");

Js.Global.setTimeout(() => stop(system), 100)|>ignore;
