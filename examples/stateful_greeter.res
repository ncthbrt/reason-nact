open Nact

let system = start()

type greetingMsg = {name: string}

let statefulGreeter = spawn(
  ~name="stateful-greeter",
  system,
  (state, {name}, ctx) => {
    let hasPreviouslyGreetedMe = state->Belt.Set.String.has(name)
    if hasPreviouslyGreetedMe {
      Js.log("Hello again " ++ name)
      state
    } else {
      Js.log("Good to meet you, " ++ name ++ ". I am the " ++ ctx.name ++ " service!")
      state->Belt.Set.String.add(name)
    }->Js.Promise.resolve
  },
  _ => Belt.Set.String.empty,
)

statefulGreeter->dispatch({name: "Erlich"})

statefulGreeter->dispatch({name: "Erlich"})

statefulGreeter->dispatch({name: "Dinesh"})
