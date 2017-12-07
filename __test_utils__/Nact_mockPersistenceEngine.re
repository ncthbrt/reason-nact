open Nact;

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external create : unit => persistenceEngine =
  "MockPersistenceEngine";