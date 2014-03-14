type e =
  | Foo
  | Bar of e

type ('st) visitor = {
  visit: 'st -> e -> 'st * e;
}

let visit v st = function
  | Foo ->
      (st, Foo)
  | Bar e ->
      let (st, e) = v.visit st e in
      (st, Bar e)

let default_visit v st e =
  visit v st e

let default ?myv () =
  let rec viz = {
    visit = (fun st e ->
        match myv with
        | Some v ->
            v st e
        | None ->
            default_visit viz st e
      );
  } in
  viz

let _ =
  let rec v =
    default ~myv:myvisit ()
    (*{ visit = myvisit }*)

  and myvisit st = function
    | Bar e ->
        let (st, e) = myvisit st e in
        (st, Bar (Bar e))

    | e ->
        visit v st e

  in

  visit v () Foo

