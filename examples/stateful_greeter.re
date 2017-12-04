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
        Js.log("Hello again " ++ name);
        Js.Promise.resolve(state)
      } else {
        Js.log("Good to meet you, " ++ name ++ ". I am the " ++ ctx.name ++ " service!");
        Js.Promise.resolve([name, ...state])
      }
    },
    []
  );

dispatch(statefulGreeter, {name: "Erlich"});

dispatch(statefulGreeter, {name: "Erlich"});

dispatch(statefulGreeter, {name: "Dinesh"});