'use strict';

var Nact = require("../src/Nact.js");
var Curry = require("bs-platform/lib/js/curry.js");
var $$String = require("bs-platform/lib/js/string.js");
var Json_decode = require("@glennsl/bs-json/src/Json_decode.bs.js");
var Json_encode = require("@glennsl/bs-json/src/Json_encode.bs.js");
var MockPersistenceEngine = require("nact/test/mock-persistence-engine");

function toPositionInAlphabet(c) {
  return c - /* "a" */97 | 0;
}

function fromPositionInAlphabet(c) {
  return c + /* "a" */97 | 0;
}

function rot13(param) {
  return $$String.map((function (c) {
                return ((c - /* "a" */97 | 0) + 13 | 0) % 26 + /* "a" */97 | 0;
              }), param);
}

function jsonDecoder(json) {
  return {
          version: Json_decode.field("version", Json_decode.$$int, json),
          text: Json_decode.field("text", Json_decode.string, json)
        };
}

function decoder(json) {
  var msg = jsonDecoder(json);
  if (msg.version === 0) {
    return {
            version: msg.version,
            text: rot13(msg.text)
          };
  } else {
    return {
            version: msg.version,
            text: msg.text
          };
  }
}

function encoder(msg) {
  return Json_encode.object_(/* :: */[
              /* tuple */[
                "version",
                msg.version
              ],
              /* :: */[
                /* tuple */[
                  "text",
                  msg.text
                ],
                /* [] */0
              ]
            ]);
}

var system = Nact.start(/* None */0, /* Some */[new MockPersistenceEngine.MockPersistenceEngine()], /* None */0, /* () */0);

var actor = Nact.spawnPersistent("da-vinci-code", /* None */0, /* None */0, /* None */0, /* None */0, /* Some */[decoder], /* None */0, /* None */0, /* Some */[encoder], system, (function (state, msg, ctx) {
        console.log(msg.text);
        var nextState = function () {
          return Promise.resolve(/* :: */[
                      msg,
                      state
                    ]);
        };
        if (ctx[/* recovering */6]) {
          return Promise.resolve(/* :: */[
                      msg,
                      state
                    ]);
        } else {
          return Curry._1(ctx[/* persist */4], msg).then(nextState);
        }
      }), /* [] */0);

Nact.Operators[/* <-< */0](actor, {
      version: 0,
      text: "uryybjbeyq"
    });

Nact.Operators[/* <-< */0](actor, {
      version: 1,
      text: "the time has come"
    });

Nact.Operators[/* <-< */0](actor, {
      version: 0,
      text: "sbewblynhtugrenaqcyraglbssha"
    });

var a = /* "a" */97;

exports.a = a;
exports.toPositionInAlphabet = toPositionInAlphabet;
exports.fromPositionInAlphabet = fromPositionInAlphabet;
exports.rot13 = rot13;
exports.jsonDecoder = jsonDecoder;
exports.decoder = decoder;
exports.encoder = encoder;
exports.system = system;
exports.actor = actor;
/* system Not a pure module */
