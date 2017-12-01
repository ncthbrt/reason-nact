open Nact;

type contactId =
  | ContactId(int);

module ContactIdCompare = {
  type t = contactId;
  let compare = (ContactId(left), ContactId(right)) => compare(left, right);
};

module ContactIdMap = Map.Make(ContactIdCompare);

type contact = {
  name: string,
  email: string
};

type contactMsg =
  | CreateContact(contact)
  | RemoveContact(contactId)
  | UpdateContact(contactId, contact)
  | FindContact(contactId);

type contactResponseMsg =
  | Success(contactId, contact)
  | NotFound(contactId);

type contactsServiceState = {
  contacts: ContactIdMap.t(contact),
  seqNumber: int
};

let createContact = ({contacts, seqNumber}, contact, ctx: ctx(_, _, _, _, _)) => {
  let contactId = ContactId(seqNumber);
  optionallyDispatch(ctx.sender, Success(contactId, contact));
  let nextContacts = ContactIdMap.add(contactId, contact, contacts);
  {contacts: nextContacts, seqNumber: seqNumber + 1}
};

let removeContact = ({contacts, seqNumber}, contactId, ctx: ctx(_, _, _, _, _)) => {
  let nextContacts = ContactIdMap.remove(contactId, contacts);
  let msg =
    if (contacts === nextContacts) {
      let contact = ContactIdMap.find(contactId, contacts);
      Success(contactId, contact)
    } else {
      NotFound(contactId)
    };
  optionallyDispatch(ctx.sender, msg);
  {contacts: nextContacts, seqNumber}
};

let updateContact = ({contacts, seqNumber}, contactId, contact, ctx: ctx(_, _, _, _, _)) => {
  let nextContacts =
    ContactIdMap.remove(contactId, contacts) |> ContactIdMap.add(contactId, contact);
  let msg =
    if (nextContacts === contacts) {
      Success(contactId, contact)
    } else {
      NotFound(contactId)
    };
  optionallyDispatch(ctx.sender, msg);
  {contacts: nextContacts, seqNumber}
};

let findContact = ({contacts, seqNumber}, contactId, ctx: ctx(_, _, _, _, _)) => {
  let msg =
    try (Success(contactId, ContactIdMap.find(contactId, contacts))) {
    | Not_found => NotFound(contactId)
    };
  optionallyDispatch(ctx.sender, msg);
  {contacts, seqNumber}
};

let system = start();

let contactsService =
  spawn(
    ~name="contacts",
    system,
    (state, msg, ctx) =>
      switch msg {
      | CreateContact(contact) => createContact(state, contact, ctx)
      | RemoveContact(contactId) => removeContact(state, contactId, ctx)
      | UpdateContact(contactId, contact) => updateContact(state, contactId, contact, ctx)
      | FindContact(contactId) => findContact(state, contactId, ctx)
      },
    {contacts: ContactIdMap.empty, seqNumber: 0}
  );

let creationResult =
  query(
    ~timeout=100,
    contactsService,
    CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"})
  );

let mapSome = (f, opt) =>
  switch opt {
  | Some(v) => f(v)
  | None => None
  };

let tryDecodeContactFromJson = (body: Js.Json.t) =>
  switch (Js.Json.decodeObject(body)) {
  | Some(obj) =>
    let possibleName = Js.Dict.get(obj, "name") |> mapSome(Js.Json.decodeString);
    let possibleEmail = Js.Dict.get(obj, "email") |> mapSome(Js.Json.decodeString);
    switch (possibleEmail, possibleName) {
    | (Some(email), Some(name)) => Some({email, name})
    | _ => None
    }
  | _ => None
  };