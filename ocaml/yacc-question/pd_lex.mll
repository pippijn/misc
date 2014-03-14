{
open Pd_par
}

rule token = parse
| ' '			{ token lexbuf }
| '\n'			{ EOS }

| "Sub"			{ SUB }
| "End"			{ END }
| "Call"		{ CALL }
| ":="			{ ASSIGN }

| ['a'-'z']+ as s	{ ID s }
| ['0'-'9']+ as i	{ INT (int_of_string i) }
| eof			{ failwith "EOF in body" }

| _ as c		{ failwith (Char.escaped c) }


{
  let token lexbuf =
    let tok = token lexbuf in
    let desc =
      match tok with
      | EOS -> "EOS"
      | SUB -> "SUB"
      | END -> "END"
      | CALL -> "CALL"
      | ASSIGN -> "ASSIGN"
      | ID id -> "ID " ^ id
      | INT i -> "INT " ^ string_of_int i
    in
    print_endline ("Lexer: " ^ desc);
    tok
}
