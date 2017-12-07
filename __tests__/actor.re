open Jest;

let mockEngine = Nact_mockPersistenceEngine.create();

describe("Expect", () => Expect.(test("toBe", () => expect(1 + 2) |> toBe(3))));