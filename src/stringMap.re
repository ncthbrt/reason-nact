module StringCompare = {
    type t = string;
    let compare = String.compare;
};

module StringMap = Map.Make(StringCompare);
include StringMap;

let fromJsMap = (jsMap: JsMap.t('value)) =>   
    Js.Array.reduce((map, (key, value)) =>  StringMap.add(key, value, map), StringMap.empty, JsMap.entries(jsMap));
