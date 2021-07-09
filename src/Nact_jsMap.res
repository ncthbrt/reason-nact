type t<'key, 'value>

type iterator<'value>

module Bindings = {
  @send external keys: t<'key, 'value> => iterator<string> = "keys"
  @val("Array.from") external toArray: iterator<'value> => array<'value> = "toArray"
}

let keys: t<'key, 'value> => array<'key> = map => map |> Bindings.keys |> Bindings.toArray
