'use strict';

var $$Map      = require("bs-platform/lib/js/map.js");
var Curry      = require("bs-platform/lib/js/curry.js");
var $$String   = require("bs-platform/lib/js/string.js");
var Nact_jsMap = require("./Nact_jsMap.js");

var StringCompare = /* module */[/* compare */$$String.compare];

var StringMap = $$Map.Make(StringCompare);

function fromJsMap(jsMap) {
  return Nact_jsMap.entries(jsMap).reduce((function (map, param) {
                return Curry._3(StringMap[/* add */3], param[0], param[1], map);
              }), StringMap[/* empty */0]);
}

var empty = StringMap[0];

var is_empty = StringMap[1];

var mem = StringMap[2];

var add = StringMap[3];

var singleton = StringMap[4];

var remove = StringMap[5];

var merge = StringMap[6];

var compare = StringMap[7];

var equal = StringMap[8];

var iter = StringMap[9];

var fold = StringMap[10];

var for_all = StringMap[11];

var exists = StringMap[12];

var filter = StringMap[13];

var partition = StringMap[14];

var cardinal = StringMap[15];

var bindings = StringMap[16];

var min_binding = StringMap[17];

var max_binding = StringMap[18];

var choose = StringMap[19];

var split = StringMap[20];

var find = StringMap[21];

var map = StringMap[22];

var mapi = StringMap[23];

exports.StringCompare = StringCompare;
exports.StringMap     = StringMap;
exports.empty         = empty;
exports.is_empty      = is_empty;
exports.mem           = mem;
exports.add           = add;
exports.singleton     = singleton;
exports.remove        = remove;
exports.merge         = merge;
exports.compare       = compare;
exports.equal         = equal;
exports.iter          = iter;
exports.fold          = fold;
exports.for_all       = for_all;
exports.exists        = exists;
exports.filter        = filter;
exports.partition     = partition;
exports.cardinal      = cardinal;
exports.bindings      = bindings;
exports.min_binding   = min_binding;
exports.max_binding   = max_binding;
exports.choose        = choose;
exports.split         = split;
exports.find          = find;
exports.map           = map;
exports.mapi          = mapi;
exports.fromJsMap     = fromJsMap;
/* StringMap Not a pure module */
