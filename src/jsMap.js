'use strict';

var $$Array   = require("bs-platform/lib/js/array.js");
var Curry     = require("bs-platform/lib/js/curry.js");
var Immutable = require("immutable-re/src/Immutable.js");

function entries(map) {
  return Curry._1(( function(map){ return Array.from(map.entries()); } ), map);
}

function keys(map) {
  return Curry._1(( function(map){ return Array.from(map.keys()); } ), map);
}

function values(map) {
  return Curry._1(( function(map){ return Array.from(map.values()); } ), map);
}

function mapValues(f, map) {
  return new Map($$Array.map((function (param) {
                    return /* tuple */[
                            param[0],
                            Curry._1(f, param[1])
                          ];
                  }), Curry._1(( function(map){ return Array.from(map.entries()); } ), map)));
}

function stringHashHelper(_hash, _str) {
  while(true) {
    var str = _str;
    var hash = _hash;
    if (str) {
      var nextHash = (hash >>> 5) + (str[0].charCodeAt(0) | 0) | 0;
      _str = str[1];
      _hash = nextHash;
      continue ;
      
    } else {
      return hash;
    }
  };
}

function stringComparator(str1, str2) {
  var result = str2.localeCompare(str1);
  if (result > 0.0) {
    return Immutable.Ordering[/* greaterThan */1];
  } else if (result === 0.0) {
    return Immutable.Ordering[/* equal */0];
  } else {
    return Immutable.Ordering[/* lessThan */2];
  }
}

function stringHash(inputString) {
  return stringHashHelper(0, $$Array.to_list(inputString.split("")));
}

function toImmutableHashMap(jsMap) {
  return Curry._1(( function(map){ return Array.from(map.entries()); } ), jsMap).reduce((function (map, param) {
                return Immutable.HashMap[/* put */32](param[0], param[1], map);
              }), Immutable.HashMap[/* emptyWith */35](stringHash, stringComparator));
}

exports.entries            = entries;
exports.keys               = keys;
exports.values             = values;
exports.mapValues          = mapValues;
exports.stringHashHelper   = stringHashHelper;
exports.stringComparator   = stringComparator;
exports.stringHash         = stringHash;
exports.toImmutableHashMap = toImmutableHashMap;
/* Immutable Not a pure module */
