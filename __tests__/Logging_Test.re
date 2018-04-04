open Js.Promise;

open Nact;

open Nact.Operators;

let (?:) = v => resolve(v);

let (>=>) = (promise1, promise2) => then_(promise2, promise1);

let (>/=>) = (promise1, promise2) => catch(promise2, promise1);

let delay: int => Js.Promise.t(unit) =
  ms =>
    Js.Promise.make((~resolve, ~reject as _) =>
      Js.Global.setTimeout(() => resolve(. (): unit), ms) |> ignore
    );

open Jest;

open ExpectJs;

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createMockPersistenceEngine : unit => persistenceEngine =
  "MockPersistenceEngine";

[@bs.deriving accessors]
type loggingSpyProtocol =
  | GetMessages(actorRef(list(Nact.Log.t)))
  | Log(Nact.Log.t);

type ctxF('parent) =
  | ContextF(ctx(ctxF('parent), 'parent) => unit);

let spawnContextExposer: actorRef('a) => actorRef(ctxF('a)) =
  system => spawnStateless(system, (ContextF(msg), ctx) => ?:(msg(ctx)));

let mockDate: unit => unit = [%bs.raw
  {|
    function() {
      var RealDate = global.Date;
      global.Date = class extends RealDate  {
       constructor(){
        return new RealDate(0);
      }
    }
  }
|}
];

describe("Log", () => {
  let system = ref(nobody());
  let spy = ref(nobody());
  let createLogger = system => {
    spy :=
      spawn(
        system,
        (state, msg, _) =>
          switch (msg) {
          | GetMessages(x) =>
            x <-< state;
            ?:state;
          | Log(log) => ?:[log, ...state]
          },
        [],
      );
    spawnAdapter(spy^, log);
  };
  beforeEach(() => system := start(~logger=createLogger, ()));
  afterEach(() => {
    stop(system^);
    system := nobody();
  });
  test("should be able to setup both the persistence engine and logger", () => {
    system :=
      start(
        ~logger=createLogger,
        ~persistenceEngine=createMockPersistenceEngine(),
        (),
      );
    pass;
  });
  describe("trace", () => {
    testPromise("should be able to invoke trace()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.trace("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Trace,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
    testPromise("should be able to invoke debug()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.debug("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Debug,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
    testPromise("should be able to invoke info()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.info("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Info,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
    testPromise("should be able to invoke warn()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.warn("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Warn,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
    testPromise("should be able to invoke error()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.error("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Error,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
    testPromise("should be able to invoke critical()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer <-< ContextF(ctx => Log.critical("testMessage", ctx.logger));
      let expectedMsg =
        Log.Message(
          Critical,
          "testMessage",
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(100)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    });
  });
  describe("error", () =>
    testPromise("should be able to invoke exception()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer
      <-< ContextF(
            ctx => Log.exception_(Failure("testMessage"), ctx.logger),
          );
      let expectedMsg =
        Log.Error(
          Failure("testMessage"),
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(50)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    })
  );
  describe("metric", () =>
    testPromise("should be able to invoke metric()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer
      <-< ContextF(
            ctx =>
              Log.metric(~name="testMessage", ~values=[|1|], ctx.logger),
          );
      let expectedMsg =
        Log.Metric(
          "testMessage",
          Js.Json.parseExn("[1]"),
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(50)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    })
  );
  describe("event", () =>
    testPromise("should be able to invoke event()", () => {
      mockDate();
      let exposer = spawnContextExposer(system^);
      exposer
      <-< ContextF(
            ctx =>
              Log.event(~name="testMessage", ~properties=[|1|], ctx.logger),
          );
      let expectedMsg =
        Log.Event(
          "testMessage",
          Js.Json.parseExn("[1]"),
          Js.Date.make(),
          ActorPath.fromReference(exposer),
        );
      delay(50)
      >=> (() => spy^ <? (getMessages, 100 * milliseconds))
      >=> (logs => ?:(expect(logs) |> toEqual([expectedMsg])));
    })
  );
});