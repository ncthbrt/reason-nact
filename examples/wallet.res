open Nact

module Transaction = {
  type t = {
    amount: int,
    createdAt: Js.Date.t,
    reference: string,
  }
}

module Wallet = {
  type id = WalletId(string)
  type t = {
    balance: int,
    id: id,
    transactions: unit => Js.Promise.t<list<Transaction.t>>,
  }
}

let transactionsQuery = (parent, id) =>
  Nact.persistentQuery(
    ~key="wallet" ++ id,
    parent,
    (state, msg) =>
      switch msg {
      | #Transaction(t) => Js.Promise.resolve(list{t, ...state})
      | _ => Js.Promise.resolve(state)
      },
    list{},
  )

let stateDecoder = (parent, json) => {
  open Json.Decode
  let id = field("id", string, json)
  {
    Wallet.id: Wallet.WalletId(id),
    balance: field("id", int, json),
    transactions: transactionsQuery(parent, id),
  }
}

let stateEncoder = ({id: WalletId(id), balance}: Wallet.t) => {
  open Json.Encode
  object_(list{("id", id |> string), ("balance", balance |> int)})
}

let spawnWallet = (walletId, parent) =>
  spawnPersistent(
    parent,
    ~key="wallet" ++ walletId,
    ~stateEncoder,
    ~stateDecoder=stateDecoder(parent),
    ~snapshotEvery=5 * messages,
    (state, msg, {recovering, persist}) =>
      switch msg {
      | #Transaction(t) =>
        (recovering ? Js.Promise.resolve() : persist(#Transaction(t))) |> Js.Promise.then_(() => {
          open Transaction
          Js.Promise.resolve({...state, balance: state.balance + t.amount})
        })
      | #GetTransactions(requestee) =>
        ignore(
          state.transactions() |> Js.Promise.then_(transactions =>
            Js.Promise.resolve(requestee->dispatch(transactions))
          ),
        )
        Js.Promise.resolve(state)
      },
    _ => {
      Wallet.balance: 0,
      id: Wallet.WalletId(walletId),
      transactions: transactionsQuery(parent, walletId),
    },
  )
