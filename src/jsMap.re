type t('value);

[@bs.new] external create: array((string, 'value)) => t('value) = "Map";
[@bs.send] external find: t('value) => string => 'value = "get";
[@bs.send] external has: t('value) => string => bool = "has";
[@bs.send] external get: t('value) => string => 'value = "get";
[@bs.send] external set: t('value) => string => 'value => t('value) = "set";

let entries: t('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.entries()); } |}](map);

let keys: t('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.keys()); } |}](map);

let values: t('value) => array((string, 'value)) = 
    map => [%bs.raw {| function(map){ return Array.from(map.values()); } |}](map);

[@bs.send] external clear: t('value) => unit = "clear";
[@bs.send] external delete: t('value) => string => bool = "delete";
[@bs.send] external forEach: t('value) => ('value => string => t('value) => unit) => unit = "forEach";
[@bs.get] external size: t('value) => int = "size";

let mapValues: ('value0 => 'value1) => t('value0) => t('value1) = 
    f => map => entries(map) |> Array.map(((k, v)) => (k, f(v))) |> create;
