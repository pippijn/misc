type expr =
  | Int of int
  | Id of string
  deriving (Show)

type stmt =
  | Call of expr
  | Assign of expr * expr
  deriving (Show)

type procedure_body = stmt list
  deriving (Show)

type procedure_declaration =
  { procedure_name : string; procedure_body : procedure_body; }
  deriving (Show)

type ev = procedure_declaration list
  deriving (Show)
