open Js.Promise;

open Nact;

let (?:) = (v) => resolve(v);

let (>=>) = (promise1, promise2) => then_(promise2, promise1);

let (>/=>) = (promise1, promise2) => catch(promise2, promise1);

open Jest;

open ExpectJs;

type loggingSpyProtocol =
  | GetMessages(actorRef(list(Nact.Log.t)))
  | Log(Nact.Log.t);

/* let createLogger = (system) => {
     let actor = spawn(system, (state, msg, ctx) => state, []);
     (actor, spawnAdapter(actor, (logMsg) => Log(logMsg)))
   }; */
describe(
  "Log",
  () => {
    describe("trace", () => ());
    describe("error", () => ());
    describe("metric", () => ())
  }
);