{
open Parent_par
}

rule token = parse
| ' '			{ token lexbuf }
| '\n'			{ EOS }

| "Sub"			{ SUB }

| ['a'-'z']+ as s	{ ID s }
| eof			{ EOF }

| _ as c		{ failwith (Char.escaped c) }


{
  let token lexbuf =
    let tok = token lexbuf in
    let desc =
      match tok with
      | EOS -> "EOS"
      | SUB -> "SUB"
      | ID id -> "ID " ^ id
      | EOF -> "EOF"
    in
    print_endline ("Lexer: " ^ desc);
    tok
}
