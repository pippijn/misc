%token EOS SUB END ASSIGN CALL
%token<string> ID
%token<int> INT

%start main
%type<Syntax.procedure_body> main
%%

main: procedure_body END SUB { $1 }

procedure_body:
  /* empty */ { [] }
| procedure_body stmt { $1 @ [$2] }

stmt:
  expr ASSIGN expr EOS { Syntax.Assign ($1, $3) }
| CALL expr EOS { Syntax.Call $2 }

expr:
  ID { Syntax.Id $1 }
| INT { Syntax.Int $1 }
