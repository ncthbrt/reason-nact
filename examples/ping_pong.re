open Nact;

let system = start();

let ping = spawnStateless(~name="ping", system, (msg, ctx) => {
    print_endline(msg);
    switch ctx.sender {
        | Some(sender) => dispatch(~sender=ctx.self, sender, ctx.name);
        | None => ()
    };       
});

let pong = spawnStateless(~name="pong", system, (msg, ctx) => {
    print_endline(msg);
    switch ctx.sender {
        | Some(sender) => dispatch(~sender=ctx.self, sender, ctx.name);
        | None => ()
    };    
});

dispatch(~sender=pong, ping, "hello");

Js.Global.setTimeout(() => stop(system), 100);