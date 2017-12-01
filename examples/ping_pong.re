open Nact;

let system = start();

let ping =
  spawnStateless(
    ~name="ping",
    system,
    (msg, ctx) => {
      print_endline(msg);
      optionallyDispatch(~sender=ctx.self, ctx.sender, ctx.name)
    }
  );

let pong =
  spawnStateless(
    ~name="pong",
    system,
    (msg, ctx) => {
      print_endline(msg);
      optionallyDispatch(~sender=ctx.self, ctx.sender, ctx.name)
    }
  );

dispatch(~sender=pong, ping, "hello");

Js.Global.setTimeout(() => stop(system), 100);