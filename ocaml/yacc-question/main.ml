let () =
  let lexbuf = Lexing.from_string "\
Sub proc
  foo := 3
  Call bar
End Sub
Sub prac
  Call proc
  foo := 3
End Sub
" in

  let ev = Parent_par.main Parent_lex.token lexbuf in
  print_endline (Show.show<Syntax.ev> ev);

  ()
