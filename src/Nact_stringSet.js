'use strict';

var $$Set = require("bs-platform/lib/js/set.js");
var Curry = require("bs-platform/lib/js/curry.js");
var $$String = require("bs-platform/lib/js/string.js");

var StringCompare = /* module */[/* compare */$$String.compare];

var StringSet = $$Set.Make(StringCompare);

function fromJsArray(jsArray) {
  return jsArray.reduce((function (set, value) {
                return Curry._2(StringSet[/* add */3], value, set);
              }), StringSet[/* empty */0]);
}

var empty = StringSet[0];

var is_empty = StringSet[1];

var mem = StringSet[2];

var add = StringSet[3];

var singleton = StringSet[4];

var remove = StringSet[5];

var union = StringSet[6];

var inter = StringSet[7];

var diff = StringSet[8];

var compare = StringSet[9];

var equal = StringSet[10];

var subset = StringSet[11];

var iter = StringSet[12];

var fold = StringSet[13];

var for_all = StringSet[14];

var exists = StringSet[15];

var filter = StringSet[16];

var partition = StringSet[17];

var cardinal = StringSet[18];

var elements = StringSet[19];

var min_elt = StringSet[20];

var max_elt = StringSet[21];

var choose = StringSet[22];

var split = StringSet[23];

var find = StringSet[24];

var of_list = StringSet[25];

exports.StringCompare = StringCompare;
exports.StringSet = StringSet;
exports.empty = empty;
exports.is_empty = is_empty;
exports.mem = mem;
exports.add = add;
exports.singleton = singleton;
exports.remove = remove;
exports.union = union;
exports.inter = inter;
exports.diff = diff;
exports.compare = compare;
exports.equal = equal;
exports.subset = subset;
exports.iter = iter;
exports.fold = fold;
exports.for_all = for_all;
exports.exists = exists;
exports.filter = filter;
exports.partition = partition;
exports.cardinal = cardinal;
exports.elements = elements;
exports.min_elt = min_elt;
exports.max_elt = max_elt;
exports.choose = choose;
exports.split = split;
exports.find = find;
exports.of_list = of_list;
exports.fromJsArray = fromJsArray;
/* StringSet Not a pure module */
