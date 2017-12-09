open Jest;

open ExpectJs;

open Nact;

/* let delay: int => Js.Promise.t(unit) =
   (ms) =>
     Js.Promise.make(
       (~resolve, ~reject: _) => {
         Js.Global.setTimeout((_) => resolve(() => ()), ms);
         ()
       }
     ); */
module StringCompare = {
  type t = string;
  let compare = String.compare;
};

module StringMap = Map.Make(StringCompare);

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createMockPersistenceEngine : unit => persistenceEngine =
  "MockPersistenceEngine";

let (>=>) = (promise1, promise2) => Js.Promise.then_(promise2, promise1);

let (>/=>) = (promise1, promise2) => Js.Promise.catch(promise2, promise1);

type statelessTestActorMsgType =
  | Echo(string)
  | Ignore;

type statefulTestActorMsgType =
  | Add(int)
  | GetTotal;

describe(
  "Stateless Actor",
  () => {
    testPromise(
      "allows queries to resolve",
      () => {
        let system = start();
        let actor =
          spawnStateless(
            system,
            ((sender, msg), _) =>
              (
                switch msg {
                | Echo(text) => dispatch(sender, text)
                | Ignore => ()
                }
              )
              |> Js.Promise.resolve
          );
        let queryPromise =
          query(~timeout=30 * milliseconds, actor, (temp) => (temp, Echo("hello")));
        queryPromise >=> ((result) => expect(result) |> toBe("hello") |> Js.Promise.resolve)
      }
    );
    testPromise(
      "shutsdown automatically after timeout",
      () => {
        let system = start();
        let actor =
          spawnStateless(
            ~shutdownAfter=10 * milliseconds,
            system,
            ((sender, msg), _) =>
              (
                switch msg {
                | Echo(text) => dispatch(sender, text)
                | Ignore => ()
                }
              )
              |> Js.Promise.resolve
          );
        let queryPromise =
          query(~timeout=30 * milliseconds, actor, (temp) => (temp, Echo("hello")));
        queryPromise >=> ((result) => expect(result) |> toBe("hello") |> Js.Promise.resolve)
      }
    );
    testPromise(
      "can be stopped",
      () => {
        let system = start();
        let actor =
          spawnStateless(
            system,
            ((sender, msg), _) =>
              (
                switch msg {
                | Echo(text) => dispatch(sender, text)
                | Ignore => ()
                }
              )
              |> Js.Promise.resolve
          );
        stop(actor);
        let queryPromise =
          query(~timeout=30 * milliseconds, actor, (temp) => (temp, Echo("hello")));
        queryPromise
        >=> ((_) => fail("Query should not be have resolved") |> Js.Promise.resolve)
        >/=> ((_) => pass |> Js.Promise.resolve)
      }
    );
    testPromise(
      "allows queries to timeout",
      () => {
        let system = start();
        let actor =
          spawnStateless(
            ~name="test1",
            system,
            ((sender, msg), _) =>
              (
                switch msg {
                | Echo(text) => dispatch(sender, text)
                | Ignore => ()
                }
              )
              |> Js.Promise.resolve
          );
        let queryPromise = query(~timeout=20 * milliseconds, actor, (temp) => (temp, Ignore));
        queryPromise
        >=> ((_) => fail("Query should not be have resolved") |> Js.Promise.resolve)
        >/=> ((_) => pass |> Js.Promise.resolve)
      }
    )
  }
);

describe(
  "Stateful Actor",
  () => {
    testPromise(
      "allows queries to resolve",
      () => {
        let system = start();
        let actor =
          spawn(
            ~name="calculator",
            system,
            (total, (sender, msg), _) =>
              (
                switch msg {
                | Add(number) => total + number
                | GetTotal =>
                  dispatch(sender, total);
                  total
                }
              )
              |> Js.Promise.resolve,
            0
          );
        let loggerActor = spawnStateless(system, (msg, _) => print_int(msg) |> Js.Promise.resolve);
        dispatch(actor, (loggerActor, Add(5)));
        dispatch(actor, (loggerActor, Add(10)));
        let queryPromise = query(~timeout=30 * milliseconds, actor, (temp) => (temp, GetTotal));
        queryPromise >=> ((result) => expect(result) |> toBe(15) |> Js.Promise.resolve)
      }
    );
    testPromise(
      "can have children",
      () => {
        let system = start();
        let spawnCalculator = (parent) =>
          spawn(
            parent,
            (total, (sender, msg), _) =>
              (
                switch msg {
                | Add(number) => total + number
                | GetTotal =>
                  dispatch(sender, total);
                  total
                }
              )
              |> Js.Promise.resolve,
            0
          );
        let parent =
          spawn(
            system,
            (children, (sender, calc, msg), ctx) => {
              let childMsg = (sender, msg);
              let calcActor =
                try (StringMap.find(calc, children)) {
                | _ => spawnCalculator(ctx.self)
                };
              dispatch(calcActor, childMsg);
              StringMap.add(calc, calcActor, children) |> Js.Promise.resolve
            },
            StringMap.empty
          );
        let loggerActor = spawnStateless(system, (msg, _) => print_int(msg) |> Js.Promise.resolve);
        dispatch(parent, (loggerActor, "a", Add(5)));
        dispatch(parent, (loggerActor, "b", Add(10)));
        dispatch(parent, (loggerActor, "b", Add(5)));
        let queryPromise =
          query(~timeout=30 * milliseconds, parent, (temp) => (temp, "b", GetTotal));
        queryPromise >=> ((result) => expect(result) |> toBe(15) |> Js.Promise.resolve)
      }
    )
  }
);

describe(
  "Stateful Actor",
  () =>
    testPromise(
      "allows queries to resolve",
      () => {
        let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
        let actor =
          spawnPersistent(
            ~key="calculator",
            ~name="calculator",
            system,
            (total, (sender, msg), _) =>
              (
                switch msg {
                | Add(number) => total + number
                | GetTotal =>
                  dispatch(sender, total);
                  total
                }
              )
              |> Js.Promise.resolve,
            0
          );
        let loggerActor = spawnStateless(system, (msg, _) => print_int(msg) |> Js.Promise.resolve);
        dispatch(actor, (loggerActor, Add(5)));
        dispatch(actor, (loggerActor, Add(10)));
        let queryPromise = query(~timeout=30 * milliseconds, actor, (temp) => (temp, GetTotal));
        queryPromise >=> ((result) => expect(result) |> toBe(15) |> Js.Promise.resolve)
      }
    )
);