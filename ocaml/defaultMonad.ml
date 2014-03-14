module type Monad = sig
  type 'a m
  val return : 'a -> 'a m
  val (>>=) : 'a m -> ('a -> 'b m) -> 'b m
  val (>>) : 'a m -> 'b m -> 'b m
end

module DefaultMonad(M :
  sig
    type 'a m
    val return : 'a -> 'a m
    val (>>=) : 'a m -> ('a -> 'b m) -> 'b m
  end) : Monad with type 'a m = 'a M.m =
struct
  open M
  let (>>) m f = m >>= fun _ -> f
  include M
end
