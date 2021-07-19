open Nact

type contactId = ContactId(int)

type contact = {
  name: string,
  email: string,
}

type contactResponseMsg =
  | Success(contact)
  | NotFound

type contactMsg =
  | CreateContact(contact)
  | RemoveContact(contactId)
  | UpdateContact(contactId, contact)
  | FindContact(contactId)

module ContactMap = {
  module ContactIdCompare = Belt.Id.MakeComparable({
    type t = contactId
    let cmp = (ContactId(left), ContactId(right)) => Pervasives.compare(left, right)
  })

  type t = Belt.Map.t<contactId, contact, ContactIdCompare.identity>

  let make = () => Belt.Map.make(~id=module(ContactIdCompare))
}

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

let system = start()

let contactsService = spawn(
  ~name="contacts",
  system,
  (state, (sender, msg), _) =>
    switch msg {
    | CreateContact(contact) => createContact(state, sender, contact)
    | RemoveContact(contactId) => removeContact(state, sender, contactId)
    | UpdateContact(contactId, contact) => updateContact(state, sender, contactId, contact)
    | FindContact(contactId) => findContact(state, sender, contactId)
    }->Js.Promise.resolve,
  _ => {contacts: ContactMap.make(), seqNumber: 0},
)

let createErlich = query(~timeout=100, contactsService, tempReference => (
  tempReference,
  CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"}),
))

let createDinesh = _ =>
  query(~timeout=100, contactsService, tempReference => (
    tempReference,
    CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"}),
  ))

let findDinsheh = ((contactId, _)) =>
  query(~timeout=100, contactsService, tempReference => (tempReference, FindContact(contactId)))

(createErlich
|> Js.Promise.then_(createDinesh)
|> Js.Promise.then_(findDinsheh)
|> Js.Promise.then_(result => {
  Js.log(result)
  Js.Promise.resolve()
}))->ignore
