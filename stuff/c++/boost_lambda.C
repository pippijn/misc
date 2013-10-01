int
main ()
{
  for_each(
    a.begin(), a.end(),
    try_catch(
      bind(foo, _1),                 // foo may throw
      catch_exception<foo_exception>(
        cout << constant("Caught foo_exception: ") 
             << "foo was called with argument = " << _1
      ),
      catch_exception<std::exception>(
        cout << constant("Caught std::exception: ") 
             << bind(&std::exception::what, _e),
        throw_exception(bind(constructor<bar_exception>(), _1))
      ),      
      catch_all(
        (cout << constant("Unknown"), rethrow())
      )
    )
  );
}
