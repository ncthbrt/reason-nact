open Nact;

open Nact.Operators;

type contactId =
  | ContactId(int);

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

module ContactIdCompare = {
  type t = contactId;
  let compare = (ContactId(left), ContactId(right)) =>
    compare(left, right);
};

module ContactIdMap = Map.Make(ContactIdCompare);

type contactsServiceState = {
  contacts: ContactIdMap.t(contact),
  seqNumber: int,
};

let createContact = ({contacts, seqNumber}, sender, contact) => {
  let contactId = ContactId(seqNumber);
  sender <-< (contactId, Success(contact));
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
  sender <-< msg;
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
  sender <-< msg;
  {contacts: nextContacts, seqNumber};
};

let findContact = ({contacts, seqNumber}, sender, contactId) => {
  let msg =
    try (contactId, Success(ContactIdMap.find(contactId, contacts))) {
    | Not_found => (contactId, NotFound)
    };
  sender <-< msg;
  {contacts, seqNumber};
};

let system = start();

let contactsService =
  spawn(
    ~name="contacts",
    system,
    (state, (sender, msg), _) =>
      (
        switch (msg) {
        | CreateContact(contact) => createContact(state, sender, contact)
        | RemoveContact(contactId) => removeContact(state, sender, contactId)
        | UpdateContact(contactId, contact) =>
          updateContact(state, sender, contactId, contact)
        | FindContact(contactId) => findContact(state, sender, contactId)
        }
      )
      |> Js.Promise.resolve,
    (_) => {contacts: ContactIdMap.empty, seqNumber: 0},
  );

let createErlich =
  query(~timeout=100, contactsService, tempReference =>
    (
      tempReference,
      CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"}),
    )
  );

let createDinesh = (_) =>
  query(~timeout=100, contactsService, tempReference =>
    (
      tempReference,
      CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"}),
    )
  );

let findDinsheh = ((contactId, _)) =>
  query(~timeout=100, contactsService, tempReference =>
    (tempReference, FindContact(contactId))
  );

let (>=>) = (promise1, promise2) => Js.Promise.then_(promise2, promise1);

createErlich
>=> createDinesh
>=> findDinsheh
>=> (
  result => {
    Js.log(result);
    Js.Promise.resolve();
  }
)|>ignore;
