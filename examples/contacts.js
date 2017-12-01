'use strict';

var $$Map                   = require("bs-platform/lib/js/map.js");
var Nact                    = require("../src/nact.js");
var Block                   = require("bs-platform/lib/js/block.js");
var Curry                   = require("bs-platform/lib/js/curry.js");
var Js_json                 = require("bs-platform/lib/js/js_json.js");
var Caml_obj                = require("bs-platform/lib/js/caml_obj.js");
var Js_primitive            = require("bs-platform/lib/js/js_primitive.js");
var Caml_builtin_exceptions = require("bs-platform/lib/js/caml_builtin_exceptions.js");

function compare(param, param$1) {
  return Caml_obj.caml_int_compare(param[0], param$1[0]);
}

var ContactIdCompare = /* module */[/* compare */compare];

var ContactIdMap = $$Map.Make(ContactIdCompare);

function createContact(param, contact, ctx) {
  var seqNumber = param[/* seqNumber */1];
  var contactId = /* ContactId */[seqNumber];
  Nact.optionallyDispatch(/* None */0, ctx[/* sender */0], /* Success */Block.__(0, [
          contactId,
          contact
        ]));
  var nextContacts = Curry._3(ContactIdMap[/* add */3], contactId, contact, param[/* contacts */0]);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */seqNumber + 1 | 0
        ];
}

function removeContact(param, contactId, ctx) {
  var contacts = param[/* contacts */0];
  var nextContacts = Curry._2(ContactIdMap[/* remove */5], contactId, contacts);
  var msg;
  if (contacts === nextContacts) {
    var contact = Curry._2(ContactIdMap[/* find */21], contactId, contacts);
    msg = /* Success */Block.__(0, [
        contactId,
        contact
      ]);
  } else {
    msg = /* NotFound */Block.__(1, [contactId]);
  }
  Nact.optionallyDispatch(/* None */0, ctx[/* sender */0], msg);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

function updateContact(param, contactId, contact, ctx) {
  var contacts = param[/* contacts */0];
  var nextContacts = Curry._3(ContactIdMap[/* add */3], contactId, contact, Curry._2(ContactIdMap[/* remove */5], contactId, contacts));
  var msg = nextContacts === contacts ? /* Success */Block.__(0, [
        contactId,
        contact
      ]) : /* NotFound */Block.__(1, [contactId]);
  Nact.optionallyDispatch(/* None */0, ctx[/* sender */0], msg);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

function findContact(param, contactId, ctx) {
  var contacts = param[/* contacts */0];
  var msg;
  try {
    msg = /* Success */Block.__(0, [
        contactId,
        Curry._2(ContactIdMap[/* find */21], contactId, contacts)
      ]);
  }
  catch (exn){
    if (exn === Caml_builtin_exceptions.not_found) {
      msg = /* NotFound */Block.__(1, [contactId]);
    } else {
      throw exn;
    }
  }
  Nact.optionallyDispatch(/* None */0, ctx[/* sender */0], msg);
  return /* record */[
          /* contacts */contacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

var system = Nact.start(/* () */0);

var contactsService = Nact.spawn(/* Some */["contacts"], system, (function (state, msg, ctx) {
        switch (msg.tag | 0) {
          case 0 : 
              return createContact(state, msg[0], ctx);
          case 1 : 
              return removeContact(state, msg[0], ctx);
          case 2 : 
              return updateContact(state, msg[0], msg[1], ctx);
          case 3 : 
              return findContact(state, msg[0], ctx);
          
        }
      }), /* record */[
      /* contacts */ContactIdMap[/* empty */0],
      /* seqNumber */0
    ]);

var creationResult = Nact.query(100, contactsService, /* CreateContact */Block.__(0, [/* record */[
          /* name */"Erlich Bachman",
          /* email */"erlich@aviato.com"
        ]]));

function mapSome(f, opt) {
  if (opt) {
    return Curry._1(f, opt[0]);
  } else {
    return /* None */0;
  }
}

function tryDecodeContactFromJson(body) {
  var match = Js_json.decodeObject(body);
  if (match) {
    var obj = match[0];
    var possibleName = mapSome(Js_json.decodeString, Js_primitive.undefined_to_opt(obj["name"]));
    var possibleEmail = mapSome(Js_json.decodeString, Js_primitive.undefined_to_opt(obj["email"]));
    if (possibleEmail && possibleName) {
      return /* Some */[/* record */[
                /* name */possibleName[0],
                /* email */possibleEmail[0]
              ]];
    } else {
      return /* None */0;
    }
  } else {
    return /* None */0;
  }
}

exports.ContactIdCompare         = ContactIdCompare;
exports.ContactIdMap             = ContactIdMap;
exports.createContact            = createContact;
exports.removeContact            = removeContact;
exports.updateContact            = updateContact;
exports.findContact              = findContact;
exports.system                   = system;
exports.contactsService          = contactsService;
exports.creationResult           = creationResult;
exports.mapSome                  = mapSome;
exports.tryDecodeContactFromJson = tryDecodeContactFromJson;
/* ContactIdMap Not a pure module */
