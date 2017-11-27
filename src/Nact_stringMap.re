module StringCompare = {
  type t = string;
  let compare = String.compare;
};

module StringMap = Map.Make(StringCompare);

include StringMap;

let fromJsMap = (jsMap: Nact_jsMap.t(string, 'value)) =>
  Js.Array.reduce(
    (map, (key, value)) => StringMap.add(key, value, map),
    StringMap.empty,
    Nact_jsMap.entries(jsMap)
  );
