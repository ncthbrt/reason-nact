open Nact;

open Nact.Operators;

module Transaction = {
  type t = {
    amount: int,
    createdAt: Js.Date.t,
    reference: string,
  };
};

module Wallet = {
  type id =
    | WalletId(string);
  type t = {
    balance: int,
    id,
    transactions: unit => Js.Promise.t(list(Transaction.t)),
  };
};

let transactionsQuery = (parent, id) =>
  Nact.persistentQuery(
    ~key="wallet" ++ id,
    parent,
    state =>
      fun
      | `Transaction(t) => Js.Promise.resolve([t, ...state])
      | _ => Js.Promise.resolve(state),
    [],
  );

let stateDecoder = (parent, json) => {
  open Json.Decode;
  let id = field("id", string, json);
  {
    Wallet.id: Wallet.WalletId(id),
    balance: field("id", int, json),
    transactions: transactionsQuery(parent, id),
  };
};

let stateEncoder = ({id: WalletId(id), balance}: Wallet.t) =>
  Json.Encode.(
    object_([
      ("id", id |> Json.Encode.string),
      ("balance", balance |> Json.Encode.int),
    ])
  );

let spawnWallet = (walletId, parent) =>
  spawnPersistent(
    parent,
    ~key="wallet" ++ walletId,
    ~stateEncoder,
    ~stateDecoder=stateDecoder(parent),
    ~snapshotEvery=5 * messages,
    (state, msg, {recovering, persist}) =>
      switch (msg) {
      | `Transaction((t: Transaction.t)) =>
        (recovering ? Js.Promise.resolve() : persist(`Transaction(t)))
        |> Js.Promise.then_(() =>
             Js.Promise.resolve({...state, balance: state.balance + t.amount})
           )
      | `GetTransactions(requestee) =>
        ignore(
          state.transactions()
          |> Js.Promise.then_(transactions =>
               Js.Promise.resolve(requestee <-< transactions)
             ),
        );
        Js.Promise.resolve(state);
      },
    (_) => {
      Wallet.balance: 0,
      id: Wallet.WalletId(walletId),
      transactions: transactionsQuery(parent, walletId),
    },
  );