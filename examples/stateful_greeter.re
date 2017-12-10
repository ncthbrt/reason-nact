open Nact;

let system = start();

type greetingMsg = {name: string};

let statefulGreeter =
  spawn(
    ~name="stateful-greeter",
    system,
    (state, {name}, ctx) => {
      let hasPreviouslyGreetedMe = List.exists((v) => v === name, state);
      if (hasPreviouslyGreetedMe) {
        print_endline("Hello again " ++ name);
        state |> Js.Promise.resolve
      } else {
        print_endline("Good to meet you, " ++ name ++ ". I am the " ++ ctx.name ++ " service!");
        [name, ...state] |> Js.Promise.resolve
      }
    },
    []
  );

open Nact.Operators;

statefulGreeter <-< {name: "Erlich"};

statefulGreeter <-< {name: "Erlich"};

statefulGreeter <-< {name: "Dinesh"};