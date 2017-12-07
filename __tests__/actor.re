open Jest;

let mockEngine = Nact_mockPersistenceEngine.create();

describe(
  "spawn",
  () => {
    Expect.(test("toBe", () => expect(1 + 2) |> toBe(3)));
    Expect.(test("toBe", () => expect(1 + 2) |> toBe(3)))
  }
);