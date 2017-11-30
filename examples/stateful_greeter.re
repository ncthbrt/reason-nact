open Nact;

let system = start();

type greetingMsg = { name: string };

let rec hasPreviouslyBeenGreeted = (name, lst) =>
  switch(name, lst) {
    | (_, []) => false;
    | (name, [head, ...tail]) => head === name ? true : hasPreviouslyBeenGreeted(name, tail);    
  };

let statefulGreeter: actorRef(_, unit) = spawn(
  ~name="stateful-greeter",
  system, 
  (state, {name}, ctx) => {
    let hasPreviouslyGreetedMe = hasPreviouslyBeenGreeted(name, state);
    if(hasPreviouslyGreetedMe) {
      Js.log("Hello again " ++ name);  
      state;
    } else {
      Js.log("Good to meet you, "++name++". I am the " ++ ctx.name ++ " service!");
      [name, ...state];
    }
  },  
  []
);

dispatch(statefulGreeter, { name: "Erlich" });
dispatch(statefulGreeter, { name: "Erlich" });
dispatch(statefulGreeter, { name: "Dinesh" });