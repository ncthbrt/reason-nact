open Jest;

open ExpectJs;

open Nact.Operators;

open Nact;

external unsafeDecoder : Js.Json.t => 'a = "%identity";

open Js.Global;

open Js.Promise;

exception ValueIsNone;

exception DecodingError;

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

let (?:) = v => resolve(v);

let (>=>) = (promise1, promise2) => then_(promise2, promise1);

let (>/=>) = (promise1, promise2) => catch(promise2, promise1);

exception NumberLessThanZeroException(int);

let resetIfNumberLessThanZeroException = (_, err, _) =>
  (
    switch (err) {
    | NumberLessThanZeroException(__) => Reset
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
    0,
  );

let spawnCalculator = parent =>
  spawn(
    parent,
    (total, (sender, msg), _) =>
      (
        switch (msg) {
        | Add(number) => total + number
        | Subtract(number) => total - number
        | GetTotal =>
          sender <-< total;
          total;
        }
      )
      |> resolve,
    0,
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
    queryPromise >=> (result => expect(result) |> toBe("hello") |> resolve);
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
      child <? (temp => (temp, GetTotal), 30 * milliseconds);
    queryPromise >=> (result => ?:(expect(result) |> toBe(12)));
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
        0,
      );
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    let queryPromise =
      query(~timeout=30 * milliseconds, actor, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toBe(15)));
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
            | _ => spawnCalculator(ctx.self)
            };
          calcActor <-< childMsg;
          ?:(StringMap.add(calc, calcActor, children));
        },
        StringMap.empty,
      );
    let loggerActor = spawnLoggerActor(system);
    parent <-< (loggerActor, "a", Add(5));
    parent <-< (loggerActor, "b", Add(10));
    parent <-< (loggerActor, "b", Add(5));
    let queryPromise =
      query(~timeout=30 * milliseconds, parent, temp =>
        (temp, "b", GetTotal)
      );
    queryPromise >=> (result => ?:(expect(result) |> toBe(15)));
  });
  testPromise("can supervise children", () => {
    let system = start();
    let parent = spawn(system, ((), (), _) => resolve(), ());
    let child =
      spawnBrokenCalculator(resetIfNumberLessThanZeroException, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toBe(12)));
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
        0,
      );
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    let queryPromise =
      query(~timeout=30 * milliseconds, actor, temp => (temp, GetTotal));
    queryPromise >=> (result => expect(result) |> toBe(15) |> resolve);
  });
  testPromise("can specify a custom decoder", () => {
    let decoder = json =>
      switch (json |> unsafeDecoder) {
      | (actor, Add(number)) => (actor, Add(number * 2))
      | x => x
      };
    let system = start(~persistenceEngine=createMockPersistenceEngine(), ());
    let actor =
      spawnPersistent(
        ~key="calculator",
        ~decoder,
        system,
        (total, (sender, msg), _) =>
          switch (msg) {
          | Add(number) => ?:(total + number)
          | Subtract(number) => ?:(total - number)
          | GetTotal =>
            total >-> sender;
            ?:total;
          },
        0,
      );
    let loggerActor =
      spawnStateless(system, (msg, _) => print_int(msg) |> resolve);
    actor <-< (loggerActor, Add(5));
    actor <-< (loggerActor, Add(10));
    let queryPromise =
      query(~timeout=30 * milliseconds, actor, temp => (temp, GetTotal));
    queryPromise >=> (result => expect(result) |> toBe(30) |> resolve);
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
        0,
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
        queryPromise >=> (result => expect(result) |> toBe(15) |> resolve);
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
        0,
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
        queryPromise >=> (result => ?:(expect(result) |> toBe(25)));
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
        true,
      );
    let child =
      spawnBrokenCalculator(resetIfNumberLessThanZeroException, parent);
    let loggerActor = spawnLoggerActor(system);
    child <-< (loggerActor, Add(5));
    child <-< (loggerActor, Add(-5));
    child <-< (loggerActor, Add(12));
    let queryPromise =
      query(~timeout=30 * milliseconds, child, temp => (temp, GetTotal));
    queryPromise >=> (result => ?:(expect(result) |> toBe(12)));
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
        false,
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
        (),
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
    let parent = spawn(system, (__, (), _) => resolve(true), true);
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
        false,
      );
    let parent =
      spawn(
        ~onCrash=dispatchToParentThatChildHasFaulted,
        grandparent,
        ((), (), _) => resolve(),
        (),
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
        false,
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
        resolve(sender <-< Nact.StringSet.cardinal(children))
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
    let parent = spawn(system, ((), (), _) => resolve(), ());
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
        resolve(sender <-< Nact.StringSet.cardinal(children))
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

describe("Adapter", () =>
  testPromise("it should foward messages to parent", () => {
    let system = start();
    let parent = spawnCalculator(system);
    let adapter = spawnAdapter(parent, msg => (nobody(), Add(msg)));
    adapter <-< 5;
    adapter <-< 5;
    delay(10)
    >=> (
      () =>
        query(~timeout=100 * milliseconds, parent, temp => (temp, GetTotal))
    )
    >=> (result => ?:(expect(result) |> toEqual(10)));
  })
);

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
    expect(pathStr) |> toBe(expectedPath);
  })
);