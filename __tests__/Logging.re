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

describe(
  "Log",
  () => {
    let system = ref(nobody());
    let spy = ref(nobody());
    let createLogger = (system) => {
      let actor = spawn(system, (state, _, _) => ?:state, []);
      spy := actor;
      spawnAdapter(actor, (logMsg) => Log(logMsg))
    };
    beforeEach(() => system := start(~logger=createLogger, ()));
    afterEach(
      () => {
        stop(system^);
        system := nobody()
      }
    );
    describe(
      "trace",
      () => {
        test("should be able to invoke trace()", () => pass);
        test("should be able to invoke debug()", () => pass);
        test("should be able to invoke info()", () => pass);
        test("should be able to invoke warn()", () => pass);
        test("should be able to invoke error()", () => pass);
        test("should be able to invoke critical()", () => pass)
      }
    );
    describe("error", () => test("should be able to invoke error()", () => pass));
    describe("metric", () => test("should be able to invoke metric()", () => pass))
  }
);