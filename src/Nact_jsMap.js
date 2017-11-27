'use strict';

var $$Array = require("bs-platform/lib/js/array.js");
var Curry   = require("bs-platform/lib/js/curry.js");

var Bindings = /* module */[];

function create(prim) {
  return new Map(prim);
}

function find(prim, prim$1) {
  return prim.find(prim$1);
}

function has(prim, prim$1) {
  return +prim.has(prim$1);
}

function get(prim, prim$1) {
  return prim.get(prim$1);
}

function set(prim, prim$1, prim$2) {
  return prim.set(prim$1, prim$2);
}

function keys(map) {
  return Array.from(map.keys());
}

function entries(map) {
  return Array.from(map.entries());
}

function values(map) {
  return Array.from(map.values());
}

function clear(prim) {
  prim.clear();
  return /* () */0;
}

function $$delete(prim, prim$1) {
  return +prim.delete(prim$1);
}

function forEach(prim, prim$1) {
  prim.forEach(prim$1);
  return /* () */0;
}

function size(prim) {
  return prim.size;
}

function mapValues(f, map) {
  return new Map($$Array.map((function (param) {
                    return /* tuple */[
                            param[0],
                            Curry._1(f, param[1])
                          ];
                  }), Array.from(map.entries())));
}

exports.Bindings  = Bindings;
exports.create    = create;
exports.find      = find;
exports.has       = has;
exports.get       = get;
exports.set       = set;
exports.keys      = keys;
exports.entries   = entries;
exports.values    = values;
exports.clear     = clear;
exports.$$delete  = $$delete;
exports.forEach   = forEach;
exports.size      = size;
exports.mapValues = mapValues;
/* No side effect */
