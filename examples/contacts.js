'use strict';

var $$Map = require("bs-platform/lib/js/map.js");
var Nact = require("../src/Nact.js");
var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Caml_primitive = require("bs-platform/lib/js/caml_primitive.js");
var Caml_builtin_exceptions = require("bs-platform/lib/js/caml_builtin_exceptions.js");

function compare(param, param$1) {
  return Caml_primitive.caml_int_compare(param[0], param$1[0]);
}

var ContactIdCompare = /* module */[/* compare */compare];

var ContactIdMap = $$Map.Make(ContactIdCompare);

function createContact(param, sender, contact) {
  var seqNumber = param[/* seqNumber */1];
  var contactId = /* ContactId */[seqNumber];
  Nact.Operators[/* <-< */0](sender, /* tuple */[
        contactId,
        /* Success */[contact]
      ]);
  var nextContacts = Curry._3(ContactIdMap[/* add */3], contactId, contact, param[/* contacts */0]);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */seqNumber + 1 | 0
        ];
}

function removeContact(param, sender, contactId) {
  var contacts = param[/* contacts */0];
  var nextContacts = Curry._2(ContactIdMap[/* remove */5], contactId, contacts);
  var msg;
  if (contacts === nextContacts) {
    var contact = Curry._2(ContactIdMap[/* find */21], contactId, contacts);
    msg = /* tuple */[
      contactId,
      /* Success */[contact]
    ];
  } else {
    msg = /* tuple */[
      contactId,
      /* NotFound */0
    ];
  }
  Nact.Operators[/* <-< */0](sender, msg);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

function updateContact(param, sender, contactId, contact) {
  var contacts = param[/* contacts */0];
  var nextContacts = Curry._3(ContactIdMap[/* add */3], contactId, contact, Curry._2(ContactIdMap[/* remove */5], contactId, contacts));
  var msg = nextContacts === contacts ? /* tuple */[
      contactId,
      /* Success */[contact]
    ] : /* tuple */[
      contactId,
      /* NotFound */0
    ];
  Nact.Operators[/* <-< */0](sender, msg);
  return /* record */[
          /* contacts */nextContacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

function findContact(param, sender, contactId) {
  var contacts = param[/* contacts */0];
  var msg;
  try {
    msg = /* tuple */[
      contactId,
      /* Success */[Curry._2(ContactIdMap[/* find */21], contactId, contacts)]
    ];
  }
  catch (exn){
    if (exn === Caml_builtin_exceptions.not_found) {
      msg = /* tuple */[
        contactId,
        /* NotFound */0
      ];
    } else {
      throw exn;
    }
  }
  Nact.Operators[/* <-< */0](sender, msg);
  return /* record */[
          /* contacts */contacts,
          /* seqNumber */param[/* seqNumber */1]
        ];
}

var system = Nact.start(/* None */0, /* None */0, /* () */0);

var contactsService = Nact.spawn(/* Some */["contacts"], /* None */0, /* None */0, system, (function (state, param, _) {
        var msg = param[1];
        var sender = param[0];
        var tmp;
        switch (msg.tag | 0) {
          case 0 : 
              tmp = createContact(state, sender, msg[0]);
              break;
          case 1 : 
              tmp = removeContact(state, sender, msg[0]);
              break;
          case 2 : 
              tmp = updateContact(state, sender, msg[0], msg[1]);
              break;
          case 3 : 
              tmp = findContact(state, sender, msg[0]);
              break;
          
        }
        return Promise.resolve(tmp);
      }), /* record */[
      /* contacts */ContactIdMap[/* empty */0],
      /* seqNumber */0
    ]);

var createErlich = Nact.query(100, contactsService, (function (tempReference) {
        return /* tuple */[
                tempReference,
                /* CreateContact */Block.__(0, [/* record */[
                      /* name */"Erlich Bachman",
                      /* email */"erlich@aviato.com"
                    ]])
              ];
      }));

function createDinesh() {
  return Nact.query(100, contactsService, (function (tempReference) {
                return /* tuple */[
                        tempReference,
                        /* CreateContact */Block.__(0, [/* record */[
                              /* name */"Dinesh Chugtai",
                              /* email */"dinesh@piedpiper.com"
                            ]])
                      ];
              }));
}

function findDinsheh(param) {
  var contactId = param[0];
  return Nact.query(100, contactsService, (function (tempReference) {
                return /* tuple */[
                        tempReference,
                        /* FindContact */Block.__(3, [contactId])
                      ];
              }));
}

function $great$eq$great(promise1, promise2) {
  return promise1.then(Curry.__1(promise2));
}

var promise1 = createErlich.then(createDinesh);

var promise1$1 = promise1.then(findDinsheh);

promise1$1.then((function (result) {
        console.log(result);
        return Promise.resolve(/* () */0);
      }));

exports.ContactIdCompare = ContactIdCompare;
exports.ContactIdMap = ContactIdMap;
exports.createContact = createContact;
exports.removeContact = removeContact;
exports.updateContact = updateContact;
exports.findContact = findContact;
exports.system = system;
exports.contactsService = contactsService;
exports.createErlich = createErlich;
exports.createDinesh = createDinesh;
exports.findDinsheh = findDinsheh;
exports.$great$eq$great = $great$eq$great;
/* ContactIdMap Not a pure module */
