open Nact;

let (>=>) = (promise1, promise2) => Js.Promise.then_(promise2, promise1);

[@bs.module "nact/test/mock-persistence-engine"] [@bs.new]
external createMockPersistenceEngine : unit => persistenceEngine =
  "MockPersistenceEngine";

module StringCompare = {
  type t = string;
  let compare = String.compare;
};

module StringMap = Map.Make(StringCompare);

type contactId =
  | ContactId(int);

module ContactIdCompare = {
  type t = contactId;
  let compare = (ContactId(left), ContactId(right)) =>
    compare(left, right);
};

module ContactIdMap = Map.Make(ContactIdCompare);

type contact = {
  name: string,
  email: string,
};

type contactResponseMsg =
  | Success(contact)
  | NotFound;

type contactMsg =
  | CreateContact(contact)
  | RemoveContact(contactId)
  | UpdateContact(contactId, contact)
  | FindContact(contactId);

type contactsServiceState = {
  contacts: ContactIdMap.t(contact),
  seqNumber: int,
};

let createContact = ({contacts, seqNumber}, sender, contact) => {
  let contactId = ContactId(seqNumber);
  dispatch(sender, (contactId, Success(contact)));
  let nextContacts = ContactIdMap.add(contactId, contact, contacts);
  {contacts: nextContacts, seqNumber: seqNumber + 1};
};

let removeContact = ({contacts, seqNumber}, sender, contactId) => {
  let nextContacts = ContactIdMap.remove(contactId, contacts);
  let msg =
    if (contacts === nextContacts) {
      let contact = ContactIdMap.find(contactId, contacts);
      (contactId, Success(contact));
    } else {
      (contactId, NotFound);
    };
  dispatch(sender, msg);
  {contacts: nextContacts, seqNumber};
};

let updateContact = ({contacts, seqNumber}, sender, contactId, contact) => {
  let nextContacts =
    ContactIdMap.remove(contactId, contacts)
    |> ContactIdMap.add(contactId, contact);
  let msg =
    if (nextContacts === contacts) {
      (contactId, Success(contact));
    } else {
      (contactId, NotFound);
    };
  dispatch(sender, msg);
  {contacts: nextContacts, seqNumber};
};

let findContact = ({contacts, seqNumber}, sender, contactId) => {
  let msg =
    try (contactId, Success(ContactIdMap.find(contactId, contacts))) {
    | Not_found => (contactId, NotFound)
    };
  dispatch(sender, msg);
  {contacts, seqNumber};
};

let system = start(~persistenceEngine=createMockPersistenceEngine(), ());

let createContactsService = (parent, userId) =>
  spawnPersistent(
    ~key="contacts" ++ userId,
    ~name=userId,
    ~shutdownAfter=15 * minutes,
    ~snapshotEvery=10 * messages,
    parent,
    (state, (sender, msg), {persist, recovering}) =>
      (
        if (! recovering) {
          persist((sender, msg));
        } else {
          Js.Promise.resolve();
        }
      )
      >=> (
        () =>
          (
            switch (msg) {
            | CreateContact(contact) => createContact(state, sender, contact)
            | RemoveContact(contactId) =>
              removeContact(state, sender, contactId)
            | UpdateContact(contactId, contact) =>
              updateContact(state, sender, contactId, contact)
            | FindContact(contactId) => findContact(state, sender, contactId)
            }
          )
          |> Js.Promise.resolve
      ),
    (_) => {contacts: ContactIdMap.empty, seqNumber: 0},
  );

let contactsService =
  spawn(
    system,
    (children, (sender, userId, msg), ctx) => {
      let potentialChild =
        try (Some(StringMap.find(userId, children))) {
        | _ => None
        };
      (
        switch (potentialChild) {
        | Some(child) =>
          dispatch(child, (sender, msg));
          children;
        | None =>
          let child = createContactsService(ctx.self, userId);
          dispatch(child, (sender, msg));
          StringMap.add(userId, child, children);
        }
      )
      |> Js.Promise.resolve;
    },
    (_) => StringMap.empty,
  );

let createErlich =
  query(~timeout=100 * milliseconds, contactsService, tempReference =>
    (
      tempReference,
      "0",
      CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"}),
    )
  );

let createDinesh = (_) =>
  query(~timeout=100 * milliseconds, contactsService, tempReference =>
    (
      tempReference,
      "1",
      CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"}),
    )
  );

open Nact.Operators;

let findContact = (userId, contactId, tempReference) => (
  tempReference,
  userId,
  FindContact(contactId),
);

let findDinsheh = ((contactId, _)) =>
  contactsService <? (findContact("1", contactId), 100 * milliseconds);

createErlich
>=> createDinesh
>=> findDinsheh
>=> (
  result => {
    Js.log(result);
    stop(system);
    Js.Promise.resolve(1);
  }
)|>ignore;
