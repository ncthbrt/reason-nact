'use strict';


var Bindings = {};

function keys(map) {
  return Array.from(map.keys());
}

exports.Bindings = Bindings;
exports.keys = keys;
/* No side effect */
