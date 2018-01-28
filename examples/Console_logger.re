open Nact;

open Js.Promise;

open Nact.Log;

let defaultTo = (default) =>
  fun
  | Some(v) => v
  | None => default;

let getLogText =
  fun
  | Message(level, text, date, actor) => {
      let pathStr = Nact.ActorPath.toString(actor);
      let dateStr = Js.Date.toUTCString(date);
      let levelStr = logLevelToString(level) |> String.uppercase;
      (levelStr, pathStr, dateStr, text)
    }
  | Error(err, date, actor) => {
      let pathStr = Nact.ActorPath.toString(actor);
      let dateStr = Js.Date.toUTCString(date);
      let json = Js.Json.stringifyAny(err) |> defaultTo("");
      ("EXCEPTION", pathStr, dateStr, json)
    }
  | Metric(name, data, date, actor) => {
      let pathStr = Nact.ActorPath.toString(actor);
      let dateStr = Js.Date.toUTCString(date);
      let json = Js.Json.stringify(data);
      ("METRIC", pathStr, dateStr, {j|{ "$name": $json }|j})
    }
  | Event(name, data, date, actor) => {
      let pathStr = Nact.ActorPath.toString(actor);
      let dateStr = Js.Date.toUTCString(date);
      let json = Js.Json.stringify(data);
      ("EVENT", pathStr, dateStr, {j|{ "$name": $json }|j})
    }
  | Unknown(payload, date, actor) => {
      let pathStr = Nact.ActorPath.toString(actor);
      let dateStr = Js.Date.toUTCString(date);
      let text = Js.Json.stringify(payload);
      ("???", pathStr, dateStr, text)
    };

let consoleLogger = (system) =>
  spawnStateless(
    ~name="console-logger",
    system,
    (msg, _) => {
      let (label, path, date, body) = getLogText(msg);
      Js.log({j|[$label, $path, $date]: $body|j});
      Js.Promise.resolve()
    }
  );

let system = start(~logger=consoleLogger, ());

let stringClassifierActor =
  spawnStateless(
    system,
    (msg, ctx) =>
      resolve
        /* strings containing mutation are evil.  */
        (
          if (Js.String.indexOf(String.lowercase(msg), "mutation") >= 0) {
            ctx.logger |> Log.event(~name="receivedEvilMessage", ~properties=msg)
          } else {
            ctx.logger |> Log.info("Received message: " ++ msg)
          }
        )
  );