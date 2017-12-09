type t('key, 'value);

type iterator('value);

module Bindings = {
  [@bs.send] external keys : t('key, 'value) => iterator(string) = "keys";
  [@bs.val "Array.from"] external toArray : iterator('value) => array('value) = "";
};

let keys: t('key, 'value) => array('key) = (map) => map |> Bindings.keys |> Bindings.toArray;