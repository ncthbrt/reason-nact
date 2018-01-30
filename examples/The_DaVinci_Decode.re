open Nact;

open Nact.Operators;

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createMockPersistenceEngine : unit => persistenceEngine =
  "MockPersistenceEngine";

let a = Char.code('a');

let toPositionInAlphabet = (c) => Char.code(c) - a;

let fromPositionInAlphabet = (c) => Char.unsafe_chr(c + a);

let rot13 = String.map((c) => (toPositionInAlphabet(c) + 13) mod 26 |> fromPositionInAlphabet);

type msg = {. "version": int, "text": string};

let jsonDecoder = (json) =>
  Json.Decode.({"version": json |> field("version", int), "text": json |> field("text", string)});

let decoder = (json) => {
  let msg = json |> jsonDecoder;
  if (msg##version == 0) {
    {"version": msg##version, "text": rot13(msg##text)}
  } else {
    {"version": msg##version, "text": msg##text}
  }
};

let encoder = (msg) =>
  Json.Encode.(object_([("version", msg##version |> int), ("text", msg##text |> string)]));

let system = start(~persistenceEngine=createMockPersistenceEngine(), ());

let actor =
  spawnPersistent(
    ~key="da-vinci-code",
    ~decoder,
    ~encoder,
    system,
    (state, msg, ctx) => {
      Js.log(msg##text);
      let nextState = () => Js.Promise.resolve([msg, ...state]);
      if (! ctx.recovering) {
        ctx.persist(msg) |> Js.Promise.then_(nextState)
      } else {
        nextState()
      }
    },
    []
  );

actor <-< {"version": 0, "text": "uryybjbeyq"};

actor <-< {"version": 1, "text": "the time has come"};

actor <-< {"version": 0, "text": "sbewblynhtugrenaqcyraglbssha"};