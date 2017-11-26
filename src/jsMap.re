type map('value);
[@bs.new] external create: array((string, 'value)) => map('value) = "Map";
[@bs.send] external find: map('value) => string => 'value = "get";
[@bs.send] external has: map('value) => string => bool = "has";
[@bs.send] external get: map('value) => string => 'value = "get";
[@bs.send] external set: map('value) => string => 'value => map('value) = "set";

let entries: map('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.entries()); } |}](map);

let keys: map('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.keys()); } |}](map);

let values: map('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.values()); } |}](map);

[@bs.send] external clear: map('value) => unit = "clear";
[@bs.send] external delete: map('value) => string => bool = "delete";
[@bs.send] external forEach: map('value) => ('value => string => map('value) => unit) => unit = "forEach";
[@bs.get] external size: map('value) => int = "size";

let mapValues: ('value0 => 'value1) => map('value0) => map('value1) = 
    f => map => entries(map) |> Array.map(((k, v)) => (k, f(v))) |> create;

let rec stringHashHelper = (hash: int, str: list(string)) =>
  switch str {
      | [head, ...tail] =>
      let nextHash = hash lsr 5 + int_of_float(Js.String.charCodeAt(0,head));
      stringHashHelper(nextHash, tail)
      | [] => hash
  };

let stringComparator = (str1: string, str2: string)  => {
  let result = Js.String.localeCompare(str1, str2);
  if(result > 0.0)  {
      Immutable.Ordering.greaterThan;
  } else if (result === 0.0) {
      Immutable.Ordering.equal;
  } else {
      Immutable.Ordering.lessThan;
  }
};

let stringHash = (inputString: string) =>   
 Js.String.split("", inputString) |> Array.to_list |> stringHashHelper(0);

let toImmutableHashMap = (jsMap: map('value)) => 
  Js.Array.reduce((map, (key, value)) => Immutable.HashMap.put(key, value, map), Immutable.HashMap.emptyWith(~hash=stringHash, ~comparator=stringComparator), entries(jsMap));

