open Nact

let system = start()

type greetingMsg = {name: string}

let greeter = spawnStateless(~name="greeter", system, ({name}, _) =>
  Js.log("Hello " ++ name)->Js.Promise.resolve
)

dispatch(greeter, {name: "Erlich Bachman"})
