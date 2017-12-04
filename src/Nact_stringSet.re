module StringCompare = {
  type t = string;
  let compare = String.compare;
};

module StringSet = Set.Make(StringCompare);

include StringSet;

let fromJsArray = (jsArray: array(string)) =>
  Js.Array.reduce((set, value) => StringSet.add(value, set), StringSet.empty, jsArray);