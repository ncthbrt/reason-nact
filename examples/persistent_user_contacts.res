open Nact

@module("nact/test/mock-persistence-engine") @new
external makeMockPersistenceEngine: unit => persistenceEngine = "MockPersistenceEngine"

type contactId = ContactId(int)

type contact = {
  name: string,
  email: string,
}

module ContactMap = {
  module ContactIdCompare = Belt.Id.MakeComparable({
    type t = contactId
    let cmp = (ContactId(left), ContactId(right)) => Pervasives.compare(left, right)
  })

  type t = Belt.Map.t<contactId, contact, ContactIdCompare.identity>

  let make = () => Belt.Map.make(~id=module(ContactIdCompare))
}

type contactResponseMsg =
  | Success(contact)
  | NotFound

type contactMsg =
  | CreateContact(contact)
  | RemoveContact(contactId)
  | UpdateContact(contactId, contact)
  | FindContact(contactId)

type contactsServiceState = {
  contacts: ContactMap.t,
  seqNumber: int,
}

let createContact = ({contacts, seqNumber}, sender, contact) => {
  let contactId = ContactId(seqNumber)
  sender->dispatch((contactId, Success(contact)))
  let nextContacts = contacts->Belt.Map.set(contactId, contact)
  {contacts: nextContacts, seqNumber: seqNumber + 1}
}

let removeContact = ({contacts, seqNumber}, sender, contactId) => {
  let nextContacts = contacts->Belt.Map.remove(contactId)
  let msg = if nextContacts == contacts {
    let contact = contacts->Belt.Map.getExn(contactId)
    (contactId, Success(contact))
  } else {
    (contactId, NotFound)
  }
  sender->dispatch(msg)
  {contacts: nextContacts, seqNumber: seqNumber}
}

let updateContact = ({contacts, seqNumber}, sender, contactId, contact) => {
  let nextContacts = contacts->Belt.Map.set(contactId, contact)
  let msg = if nextContacts == contacts {
    (contactId, Success(contact))
  } else {
    (contactId, NotFound)
  }
  sender->dispatch(msg)
  {contacts: nextContacts, seqNumber: seqNumber}
}

let findContact = ({contacts, seqNumber}, sender, contactId) => {
  let msg = try {
    (contactId, Success(contacts->Belt.Map.getExn(contactId)))
  } catch {
  | Not_found => (contactId, NotFound)
  }
  sender->dispatch(msg)
  {contacts: contacts, seqNumber: seqNumber}
}

let system = start(~persistenceEngine=makeMockPersistenceEngine(), ())

let makeContactsService = (parent, userId) =>
  spawnPersistent(
    ~key="contacts" ++ userId,
    ~name=userId,
    ~shutdownAfter=15 * minutes,
    ~snapshotEvery=10 * messages,
    parent,
    (state, (sender, msg), {persist, recovering}) =>
      (recovering ? Js.Promise.resolve() : persist((sender, msg))) |> Js.Promise.then_(() => {
        switch msg {
        | CreateContact(contact) => createContact(state, sender, contact)
        | RemoveContact(contactId) => removeContact(state, sender, contactId)
        | UpdateContact(contactId, contact) => updateContact(state, sender, contactId, contact)
        | FindContact(contactId) => findContact(state, sender, contactId)
        }->Js.Promise.resolve
      }),
    _ => {contacts: ContactMap.make(), seqNumber: 0},
  )

let contactsService = spawn(
  system,
  (children, (sender, userId, msg), ctx) => {
    switch children->Belt.Map.String.get(userId) {
    | Some(child) =>
      child->dispatch((sender, msg))
      children
    | None =>
      let child = makeContactsService(ctx.self, userId)
      child->dispatch((sender, msg))
      children->Belt.Map.String.set(userId, child)
    }->Js.Promise.resolve
  },
  _ => Belt.Map.String.empty,
)

let createErlich = query(~timeout=100, contactsService, tempReference => (
  tempReference,
  "0",
  CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"}),
))

let createDinesh = _ =>
  query(~timeout=100, contactsService, tempReference => (
    tempReference,
    "1",
    CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"}),
  ))

let findContact = (userId, contactId, tempReference) => (
  tempReference,
  userId,
  FindContact(contactId),
)

let findDinsheh = ((contactId, _)) =>
  query(~timeout=100, contactsService, findContact("1", contactId))

(createErlich
|> Js.Promise.then_(createDinesh)
|> Js.Promise.then_(findDinsheh)
|> Js.Promise.then_(result => {
  Js.log(result)
  stop(system)
  Js.Promise.resolve()
}))->ignore
