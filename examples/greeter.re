open Nact;

let system = start();

type greetingMsg = { name: string };

let greeter: actorRef(_, unit) = spawnStateless(
  ~name = "greeter",
  system,
  ({ name }, _) => print_endline("Hello "++ name)
);

dispatch(greeter, { name: "Erlich Bachman" });