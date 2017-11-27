type t('key, 'value);
type iterator('value);

module Bindings {
    [@bs.new] external create: array(('key, 'value)) => t('key, 'value) = "Map";
    [@bs.send] external find: t('key, 'value) => string => 'value = "find";
    [@bs.send] external has: t('key,'value) => string => bool = "has";
    [@bs.send] external get: t('key, 'value) => string => 'value = "get";
    [@bs.send] external set: t('key, 'value) => string => 'value => t('key, 'value) = "set";
    [@bs.send] external keys: t('key, 'value) => iterator(string) = "keys";
    [@bs.send] external entries: t('key, 'value) => iterator(('key, 'value)) = "entries";
    [@bs.send] external values: t('key, 'value) => iterator('value) = "values";    
    [@bs.send] external clear: t('key, 'value) => unit = "clear";
    [@bs.send] external delete: t('key, 'value) => string => bool = "delete";
    [@bs.send] external forEach: t('key, 'value) => ('value => string => t('key, 'value) => unit) => unit = "forEach";
    [@bs.get] external size: t('key, 'value) => int = "size";    
    [@bs.val "Array.from"] external toArray: iterator('value) => array('value) = "";
};

let create = Bindings.create;
let find = Bindings.find;
let has = Bindings.has;
let get = Bindings.get;
let set = Bindings.set;
let keys: t('key, 'value) => array('key) = 
    map => map |> Bindings.keys |> Bindings.toArray;
let entries: t('key, 'value) => array(('key, 'value)) = 
    map => Bindings.entries(map) |> Bindings.toArray;
let values: t('key, 'value) => array('value) = 
    map => map |> Bindings.values |> Bindings.toArray; 

let clear = Bindings.clear;

let delete = Bindings.delete;
let forEach = Bindings.forEach;
let size = Bindings.size;

let mapValues: ('value0 => 'value1) => t('key, 'value0) => t('key, 'value1) = 
    f => map => map |> entries |> Array.map(((k, v)) => (k, f(v))) |> create;
