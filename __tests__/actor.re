open Jest;
/* 
open Nact;

let system = start();

let ping = spawnStateless(~name="ping", system, (msg, ctx) => {
    print_endline(msg);
    switch ctx.sender {
        | None => print_endline("Saying goodbye now");
        | Some(sender) => dispatch(~sender=ctx.self, sender, ctx.name);
    };    
});

let pong = spawnStateless(~name="pong", system, (msg, ctx) => {
    print_endline(msg);
    switch ctx.sender {
        | None => print_endline("Saying goodbye now");
        | Some(sender) => dispatch(~sender=ctx.self, sender, ctx.name);
    }
});

dispatch(~sender=pong, ping, "hello");

Js.Global.setTimeout(() => stop(system), 1000); */


describe("Expect", () => {
    open Expect;
  
    test("toBe", () => expect(1 + 2) |> toBe(3))
});
