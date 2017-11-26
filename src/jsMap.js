'use strict';

var $$Array = require("bs-platform/lib/js/array.js");
var Curry   = require("bs-platform/lib/js/curry.js");

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

exports.entries   = entries;
exports.keys      = keys;
exports.values    = values;
exports.mapValues = mapValues;
/* No side effect */
