open Jest;

open Expect;

open Nact.Operators;

open Nact;

external unsafeDecoder : Js.Json.t => 'msg = "%identity";

open Js.Global;

open Js.Promise;

exception ValueIsNone;

exception DecodingError;

let initialState = (value, _) => value;

let raiseIfNone = value =>
  switch (value) {
  | Some(x) => x
  | None => raise(ValueIsNone)
  };

let delay: int => Js.Promise.t(unit) =
  ms =>
    Js.Promise.make((~resolve, ~reject as _) =>
      setTimeout(() => resolve(. (): unit), ms) |> ignore
    );

module StringCompare = {
  type t = string;
  let compare = String.compare;
};

module StringMap = Map.Make(StringCompare);

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createMockPersistenceEngine : unit => persistenceEngine =
  "MockPersistenceEngine";

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createPersistenceEngineWithData : 'a => persistenceEngine =
  "MockPersistenceEngine";

let toPersistentEvents = (key, events) =>
  events
  |> Belt.Array.mapWithIndex(_, (i, e) =>
       {"data": e, "sequenceNumber": i + 1, "key": key, "createdAt": i}
     );

let (?:) = v => resolve(v);

let (>=>) = (promise1, promise2) => then_(promise2, promise1);

let (>/=>) = (promise1, promise2) => catch(promise2, promise1);

exception NumberLessThanZeroException(int);

let resetIfNumberLessThanZeroException = (_, err, _) =>
  (
    switch (err) {
    | NumberLessThanZeroException(_) => Reset
    | _ => Stop
    }
  )
  |> resolve;

type statefulTestActorMsgType =
  | GetTotal
  | Add(int)
  | Subtract(int);

let spawnBrokenCalculator = (policy, parent) =>
  spawn(
    ~onCrash=policy,
    parent,
    (total, (sender, msg), _) =>
      switch (msg) {
      | Add(number) when number > 0 => resolve(total + number)
      | Add(number) => raise(NumberLessThanZeroException(number))
      | Subtract(number) => resolve(total - number)
      | GetTotal =>
        total >-> sender;
        resolve(total);
      },
    (_) => 0,
  );

let spawnCalculator = (~name="einstein", parent) =>
  spawn(
    parent,
    ~name,
    (total, (sender, msg), _) =>
      (
        switch (msg) {
        | Add(number) => total + number
        | Subtract(number) => total - number
        | GetTotal =>
          Nact.dispatch(sender, total);
          total;
        }
      )
      |> resolve,
    (_) => 0,
  );

type statelessTestActorMsgType =
  | Echo(string)
  | Ignore;

type faultedStatelessTestActorMsgType =
  | Reflect(string)
  | Raise;

type supervisionMsg('a) =
  | HasChildFaulted(actorRef('a))
  | ChildHasFaulted;

exception TragicException;

let spawnLoggerActor = parent =>
  spawnStateless(parent, (msg, _) => Js.log(msg) |> resolve);

let echoHello = temp => (temp, Echo("hello"));

describe("Stateless Actor", () => {
  testPromise("allows queries to resolve", () => {
    let system = start();
    let actor =
      spawnStateless(system, ((sender, msg), _) =>
        ?:(
          switch (msg) {
          | Echo(text) => text >-> sender
          | Ignore => ()
          }
        )
      );
    let queryPromise = query(~timeout=30 * milliseconds, actor, echoHello);
    queryPromise
    >=> (result => expect(result) |> toEqual("hello") |> resolve);
  });
  test("can be created with name", () => {
    let system = start();
    let actor =
      spawnStateless(~name="albert", system, ((sender, msg), _) =>
        ?:(
          switch (msg) {
          | Echo(text) => text >-> sender
          | Ignore => ()
          }
        )
      );
    let path = Nact.ActorPath.fromReference(actor);
    expect(Nact.ActorPath.parts(path)) |> toEqual(["albert"]);
  });
  testPromise("shuts down automatically after timeout", () => {
    let system = start();
    let actor =
      spawnStateless(
        ~shutdownAfter=10 * milliseconds, system, ((sender, msg), _) =>
        ?:(
          switch (msg) {
          | Echo(text) => sender <-< text
          | Ignore => ()
          }
        )
      );
    delay(20 * milliseconds)
    >=> (
      (_) =>
        query(~timeout=30 * milliseconds, actor, echoHello)
        >=> ((_) => fail("Query should not be have resolved") |> resolve)
        >/=> ((_) => pass |> resolve)
    );
  });
  testPromise("can be stopped", () => {
    let system = start();
    let actor =
      spawnStateless(system, ((sender, msg), _) =>
        ?:(
          switch (msg) {
          | Echo(text) => text >-> sender
          | Ignore => ()
          }
        )
      );
    stop(actor);
    let queryPromise = actor <? (echoHello, 30 * milliseconds);
    queryPromise
    >=> ((_) => fail("Query should not be have resolved") |> resolve)
    >/=> ((_) => pass |> resolve);
  });
  testPromise("allows queries to timeout", () => {
    let system = start();
    let actor =
      spawnStateless(~name="test1", system, ((sender, msg), _) =>
        ?:(
          switch (msg) {
          | Echo(text) => sender <-< text
          | Ignore => ()
          }
        )
      );
    let queryPromise =
      query(~timeout=20 * milliseconds, actor, temp => (temp, Ignore));
    queryPromise
    >=> ((_) => fail("Query should not be have resolved") |> resolve)
    >/=> ((_) => pass |> resolve);
  });
  testPromise("can supervise children", () => {
    let system = start();
    let parent = spawnStateless(system, ((), _) => resolve());
    let child =
      spawnBrokenCalculator(resetIfNumberLessThanZeroException, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise =
      child <? (temp => (temp, GetTotal), 50 * milliseconds);
    queryPromise >=> (result => ?:(expect(result) |> toEqual(12)));
  });
  testPromise("does not terminate even after throwing an exception", () => {
    let system = start();
    let child =
      spawnStateless(system, ((sender, msg), _) =>
        switch (msg) {
        | Reflect(text) => ?:(sender <-< text)
        | Raise => raise(TragicException)
        }
      );
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Raise);
    delay(30)
    >=> (
      (_) => {
        let queryPromise =
          query(~timeout=30 * milliseconds, child, temp =>
            (temp, Reflect("hello"))
          );
        queryPromise >=> (result => ?:(expect(result) |> toEqual("hello")));
      }
    );
  });
});

describe("Stateful Actor", () => {
  testPromise("allows queries to resolve", () => {
    let system = start();
    let actor =
      spawn(
        ~name="calculator",
        system,
        (total, (sender, msg), _) =>
          ?:(
            switch (msg) {
            | Add(number) => total + number
            | Subtract(number) => total - number
            | GetTotal =>
              sender <-< total;
              total;
            }
          ),
        initialState(0),
      );
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    let queryPromise =
      query(~timeout=30 * milliseconds, actor, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toEqual(15)));
  });
  testPromise("can have children", () => {
    let system = start();
    let parent =
      spawn(
        system,
        (children, (sender, calc, msg), ctx) => {
          let childMsg = (sender, msg);
          let calcActor =
            try (StringMap.find(calc, children)) {
            | _ =>
              spawnCalculator(
                ~name="winterfrost" ++ string_of_int(Random.int(100000000)),
                ctx.self,
              )
            };
          calcActor <-< childMsg;
          ?:(StringMap.add(calc, calcActor, children));
        },
        initialState(StringMap.empty),
      );
    let loggerActor = spawnLoggerActor(system);
    parent <-< (loggerActor, "a", Add(5));
    parent <-< (loggerActor, "b", Add(10));
    parent <-< (loggerActor, "b", Add(5));
    let queryPromise =
      query(~timeout=30 * milliseconds, parent, temp =>
        (temp, "b", GetTotal)
      );
    queryPromise >=> (result => ?:(expect(result) |> toEqual(15)));
  });
  testPromise("can supervise children", () => {
    let system = start();
    let parent = spawn(system, ((), (), _) => resolve(), initialState());
    let child =
      spawnBrokenCalculator(resetIfNumberLessThanZeroException, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toEqual(12)));
  });
});

describe("System", () => {
  test("Can name system", () => {
    let system = start(~name="albert", ());
    expect(ActorPath.systemName(ActorPath.fromReference(system)))
    |> toEqual("albert");
  });
  test("Can name system and add persistence plugin", () => {
    let result =
      start(
        ~name="albert",
        ~persistenceEngine=createMockPersistenceEngine(),
        (),
      );
    expect(result) |> ExpectJs.toBeTruthy;
  });
});

describe("Persistent Actor", () => {
  testPromise("allows queries to resolve", () => {
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let actor =
      spawnPersistent(
        ~key="calculator",
        system,
        (total, (sender, msg), _) =>
          ?:(
            switch (msg) {
            | Add(number) => total + number
            | Subtract(number) => total - number
            | GetTotal =>
              total >-> sender;
              total;
            }
          ),
        initialState(0),
      );
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    let queryPromise =
      query(~timeout=30 * milliseconds, actor, temp => (temp, GetTotal));
    queryPromise >=> (result => expect(result) |> toEqual(15) |> resolve);
  });
  testPromise("can specify a custom decoder", () => {
    let decoder = json =>
      switch (json |> unsafeDecoder) {
      | (actor, Add(number)) => (actor, Add(number * 2))
      | x => x
      };
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let actorF = () =>
      spawnPersistent(
        ~key="calculator",
        ~decoder,
        ~encoder=id => Obj.magic(id),
        system,
        (total, (sender, msg), {recovering, persist}) =>
          switch (msg) {
          | Add(number) =>
            (recovering ? resolve() : persist((Nact.nobody(), msg)))
            >=> (() => resolve(total + number))
          | Subtract(number) =>
            (recovering ? resolve() : persist((Nact.nobody(), msg)))
            >=> (() => resolve(total - number))
          | GetTotal =>
            total >-> sender;
            ?:total;
          },
        initialState(0),
      );
    let actor = actorF();
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    delay(10)
    >=> (
      () => {
        Nact.stop(actor);
        let actor = actorF();
        let queryPromise =
          query(~timeout=50 * milliseconds, actor, temp => (temp, GetTotal));
        queryPromise >=> (result => expect(result) |> toEqual(30) |> resolve);
      }
    );
  });
  testPromise("automatically snapshots", () => {
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let spawnActor = () =>
      spawnPersistent(
        ~key="calculator",
        ~name="calculator",
        ~snapshotEvery=3 * messages,
        system,
        (total, (sender, msg), ctx) =>
          switch (msg) {
          | Add(number) =>
            /* Don't add if recovering we want to test snapshotting in particular */
            let numberToAdd = ctx.recovering ? 0 : number;
            ctx.persist((sender, msg))
            >=> ((_) => resolve(total + numberToAdd));
          | Subtract(number) => ?:(total - number)
          | GetTotal =>
            total >-> sender;
            ?:total;
          },
        initialState(0),
      );
    let actorInstance1 = spawnActor();
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actorInstance1 <-< (loggerActor, Add(5));
    actorInstance1 <-< (loggerActor, Add(10));
    actorInstance1 <-< (loggerActor, Add(10));
    delay(30)
    >=> (
      (_) => {
        stop(actorInstance1);
        let actorInstance2 = spawnActor();
        let queryPromise =
          query(~timeout=30 * milliseconds, actorInstance2, temp =>
            (temp, GetTotal)
          );
        queryPromise >=> (result => expect(result) |> toEqual(25) |> resolve);
      }
    );
  });
  testPromise("can persist events", () => {
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let spawnActor = () =>
      spawnPersistent(
        ~key="calculator",
        ~name="calculator",
        system,
        (total, (sender, msg), ctx) =>
          switch (msg) {
          | Add(number) =>
            /* Don't add if recovering we want to test snapshotting in particular */
            ctx.persist((sender, msg)) >=> ((_) => resolve(total + number))
          | Subtract(number) =>
            ctx.persist((sender, msg)) >=> ((_) => resolve(total - number))
          | GetTotal =>
            total >-> sender;
            ?:total;
          },
        initialState(0),
      );
    let actorInstance1 = spawnActor();
    let loggerActor = spawnStateless(system, (msg, _) => ?:(print_int(msg)));
    actorInstance1 <-< (loggerActor, Add(5));
    actorInstance1 <-< (loggerActor, Add(10));
    actorInstance1 <-< (loggerActor, Add(10));
    delay(30)
    >=> (
      (_) => {
        stop(actorInstance1);
        let actorInstance2 = spawnActor();
        let queryPromise =
          query(~timeout=30 * milliseconds, actorInstance2, temp =>
            (temp, GetTotal)
          );
        queryPromise >=> (result => ?:(expect(result) |> toEqual(25)));
      }
    );
  });
  testPromise("can supervise children", () => {
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let parent =
      spawnPersistent(
        ~key="parent",
        system,
        (__, (), _) => resolve(true),
        initialState(true),
      );
    let child =
      spawnBrokenCalculator(resetIfNumberLessThanZeroException, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toEqual(12)));
  });
  testPromise("raises fault after throwing an exception", () => {
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let dispatchToParentThatChildHasFaulted = (_, _, ctx) => {
      ctx.parent <-< ChildHasFaulted;
      resolve(Stop);
    };
    let parent =
      spawn(
        system,
        (hasFaulted, msg, _) =>
          resolve(
            switch (msg) {
            | HasChildFaulted(sender) =>
              sender <-< hasFaulted;
              hasFaulted;
            | ChildHasFaulted => true
            },
          ),
        initialState(false),
      );
    let child =
      spawnPersistent(
        ~onCrash=dispatchToParentThatChildHasFaulted,
        ~key="test-child",
        parent,
        ((), (sender, msg), _) =>
          ?:(
            switch (msg) {
            | Reflect(text) => sender <-< text
            | Raise => raise(TragicException)
            }
          ),
        initialState(),
      );
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Raise);
    delay(30)
    >=> (
      (_) => {
        let queryPromise =
          query(~timeout=30 * milliseconds, parent, temp =>
            HasChildFaulted(temp)
          );
        queryPromise >=> (result => ?:(expect(result) |> toEqual(true)));
      }
    );
  });
});

describe("Persistent Query", () => {
  testPromise("correctly replays events", () => {
    let system =
      start(
        ~persistenceEngine=
          createPersistenceEngineWithData(
            Js.Dict.fromList([
              (
                "calculator",
                toPersistentEvents(
                  "calculator",
                  [|`Add(10), `Subtract(2), `Add(3)|],
                ),
              ),
            ]),
          ),
        (),
      );
    let query =
      persistentQuery(
        ~key="calculator",
        system,
        (total, msg) =>
          resolve(
            switch (msg) {
            | `Add(number) => total + number
            | `Subtract(number) => total - number
            },
          ),
        0,
      );
    query() >=> (result => expect(result) |> toEqual(11) |> resolve);
  });
  testPromise("can specify a custom decoder", () => {
    let decoder = json =>
      switch (json |> unsafeDecoder) {
      | `Add(number) => `Add(number * 2)
      | x => x
      };
    let system =
      start(
        ~persistenceEngine=
          createPersistenceEngineWithData(
            Js.Dict.fromList([
              (
                "calculator",
                toPersistentEvents(
                  "calculator",
                  [|`Add(10), `Subtract(2), `Add(3)|],
                ),
              ),
            ]),
          ),
        (),
      );
    let query =
      persistentQuery(
        ~key="calculator",
        ~decoder,
        ~encoder=id => Obj.magic(id),
        system,
        total =>
          fun
          | `Add(number) => resolve(total + number)
          | `Subtract(number) => resolve(total - number),
        0,
      );
    query() >=> (result => expect(result) |> toEqual(24) |> resolve);
  });
  testPromise("rejects after throwing an exception", () => {
    let system =
      start(
        ~persistenceEngine=
          createPersistenceEngineWithData(
            Js.Dict.fromList([
              (
                "calculator",
                toPersistentEvents(
                  "calculator",
                  [|`Add(10), `Subtract(2), `Add(3)|],
                ),
              ),
            ]),
          ),
        (),
      );
    let query =
      persistentQuery(
        ~key="calculator",
        system,
        ((), _) => raise(TragicException),
        (),
      );
    query()
    >=> (() => resolve(fail("This should have thrown")))
    >/=> ((_) => resolve(pass));
  });
});

describe("useStatefulSupervisionPolicy", () =>
  testPromise("can be used to construct stateful supervision policies", () => {
    let system = start();
    let resetIfFailureHasOcurredMoreThanOnce =
      useStatefulSupervisionPolicy(
        (_, _, state, __) => (
          state + 1,
          resolve(state > 0 ? Reset : Resume),
        ),
        0,
      );
    let parent = spawn(system, (__, (), _) => resolve(true), (_) => true);
    let child =
      spawnBrokenCalculator(resetIfFailureHasOcurredMoreThanOnce, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise1 =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    child <-< (loggerActor, Add(-1));
    let queryPromise2 =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    let resultPromise = Js.Promise.all([|queryPromise1, queryPromise2|]);
    resultPromise >=> (result => ?:(expect(result) |> toEqual([|17, 0|])));
  })
);

describe("supervision policy", () => {
  testPromise("can escalate", () => {
    let escalate = (_, _, _) => resolve(Escalate);
    let dispatchToParentThatChildHasFaulted = (_, _, ctx) => {
      ctx.parent <-< ChildHasFaulted;
      resolve(Stop);
    };
    let system = start();
    let grandparent =
      spawn(
        system,
        (grandchildHasFaulted, msg, _) =>
          resolve(
            switch (msg) {
            | ChildHasFaulted => true
            | HasChildFaulted(sender) =>
              sender <-< grandchildHasFaulted;
              grandchildHasFaulted;
            },
          ),
        initialState(false),
      );
    let parent =
      spawn(
        ~onCrash=dispatchToParentThatChildHasFaulted,
        grandparent,
        ((), (), _) => resolve(),
        (_) => (),
      );
    let child = spawnBrokenCalculator(escalate, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(-5));
    delay(30)
    >=> (
      (_) => {
        let queryPromise =
          query(~timeout=100 * milliseconds, grandparent, temp =>
            HasChildFaulted(temp)
          );
        queryPromise >=> (result => ?:(expect(result) |> toEqual(true)));
      }
    );
  });
  testPromise("can resume", () => {
    let resume = (_, _, __) => resolve(Resume);
    let system = start();
    let child = spawnBrokenCalculator(resume, system);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(7));
    let queryPromise =
      query(~timeout=100 * milliseconds, child, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toEqual(12)));
  });
  testPromise("can stop", () => {
    let stop = (_, _, ctx) => {
      ctx.parent <-< ChildHasFaulted;
      resolve(Stop);
    };
    let system = start();
    let parent =
      spawn(
        system,
        (grandchildHasFaulted, msg, _) =>
          switch (msg) {
          | ChildHasFaulted => ?:true
          | HasChildFaulted(sender) =>
            sender <-< grandchildHasFaulted;
            ?:grandchildHasFaulted;
          },
        initialState(false),
      );
    let child = spawnBrokenCalculator(stop, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(5));
    delay(40)
    >=> (
      () =>
        query(~timeout=100 * milliseconds, parent, temp =>
          HasChildFaulted(temp)
        )
        >=> (result => ?:(expect(result) |> toEqual(true)))
    );
  });
  testPromise("can stopAll", () => {
    let stopAll = (_, _, _) => resolve(StopAll);
    let system = start();
    let parent =
      spawnStateless(system, (sender, {children}) =>
        resolve(sender <-< Belt.Set.String.size(children))
      );
    let child1 = spawnBrokenCalculator(stopAll, parent);
    let child2 = spawnBrokenCalculator(stopAll, parent);
    let loggerActor = spawnLoggerActor(system);
    child2 <-< (loggerActor, Add(5));
    child1 <-< (loggerActor, Add(-5));
    delay(30)
    >=> (
      (_) =>
        query(~timeout=100 * milliseconds, parent, temp => temp)
        >=> (result => ?:(expect(result) |> toEqual(0)))
    );
  });
  testPromise("can reset", () => {
    let reset = (_, _, _) => resolve(Reset);
    let system = start();
    let parent = spawn(system, ((), (), _) => resolve(), initialState());
    let child = spawnBrokenCalculator(reset, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(7));
    query(~timeout=100 * milliseconds, child, temp => (temp, GetTotal))
    >=> (result => ?:(expect(result) |> toEqual(7)));
  });
  testPromise("can resetAll", () => {
    let resetAll = (_, _, _) => resolve(ResetAll);
    let system = start();
    let parent =
      spawnStateless(system, (sender, {children}) =>
        resolve(sender <-< Belt.Set.String.size(children))
      );
    let child1 = spawnBrokenCalculator(resetAll, parent);
    let child2 = spawnBrokenCalculator(resetAll, parent);
    let loggerActor = spawnLoggerActor(system);
    child1 <-< (loggerActor, Add(5));
    child2 <-< (loggerActor, Add(5));
    child1 <-< (loggerActor, Add(-5));
    delay(30)
    >=> (
      (_) => {
        child1 <-< (loggerActor, Add(7));
        child2 <-< (loggerActor, Add(7));
        let queryPromise1 =
          query(~timeout=100 * milliseconds, child1, temp =>
            (temp, GetTotal)
          );
        let queryPromise2 =
          query(~timeout=100 * milliseconds, child2, temp =>
            (temp, GetTotal)
          );
        let resultPromise = Js.Promise.all([|queryPromise1, queryPromise2|]);
        resultPromise >=> (result => ?:(expect(result) |> toEqual([|7, 7|])));
      }
    );
  });
});

describe("Adapter", () => {
  testPromise("it should foward messages to parent", () => {
    let system = start();
    let parent = spawnCalculator(~name="fritz", system);
    let adapter = spawnAdapter(parent, msg => (nobody(), Add(msg)));
    adapter <-< 5;
    adapter <-< 5;
    delay(10)
    >=> (
      () =>
        query(~timeout=100 * milliseconds, parent, temp => (temp, GetTotal))
    )
    >=> (result => ?:(expect(result) |> toEqual(10)));
  });
  test("it should be able to be named", () => {
    let system = start();
    let parent = spawnCalculator(~name="einstein", system);
    let adapter =
      spawnAdapter(~name="albert", parent, msg => (nobody(), Add(msg)));
    let path = Nact.ActorPath.fromReference(adapter);
    expect(Nact.ActorPath.parts(path)) |> toEqual(["einstein", "albert"]);
  });
});

exception QueryShouldNeverResolve;

describe("Nobody", () => {
  test("can dispatch to nobody", () => {
    let nobody: actorRef(string) = nobody();
    nobody <-< "hello";
    pass;
  });
  testPromise("query is rejected when dispatching to nobody", () => {
    let nobody: actorRef(string) = nobody();
    let query = nobody <? ((_) => "hello", 10 * milliseconds);
    query
    >=> (() => Js.Promise.reject(QueryShouldNeverResolve))
    >/=> ((_) => ?:pass);
  });
});

describe("ActorPath", () =>
  test("toString() should correctly format actor path", () => {
    let system = start();
    let systemName = ActorPath.systemName(system |> ActorPath.fromReference);
    let expectedPath = "system:" ++ systemName ++ "//name";
    let actor = spawnStateless(system, ~name="name", (_, _) => ?:());
    let pathStr = ActorPath.fromReference(actor) |> ActorPath.toString;
    expect(pathStr) |> toEqual(expectedPath);
  })
);
