%{
type menhir_env_hack = { _menhir_lexbuf : Lexing.lexbuf }
let _menhir_env = { _menhir_lexbuf = Lexing.from_function (fun _ _ -> assert false) }
%}

%token EOF EOS SUB
%token<string> ID

%start main
%type<Syntax.ev> main
%%

main: procedure_declarations EOF { $1 }
;

procedure_declarations:
  /* empty */ { [] }
| procedure_declarations procedure_declaration { $1 @ [$2] }
;

procedure_declaration:
SUB procedure_name EOS EOS
{ Syntax.({ procedure_name = fst $2; procedure_body = snd $2; }) }
;

procedure_name: ID {
  print_endline "pd start";
  let lexbuf = _menhir_env._menhir_lexbuf in
  let body = Pd_par.main Pd_lex.token lexbuf in
  print_endline "pd end";
  ($1, body)
}
;
