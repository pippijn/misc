struct visitor;

struct node
{
  virtual ~node () { }
  virtual void accept (visitor *) = 0;
};

struct node1 : node
{
  void accept (visitor *v) { v->visit (this); }
  int value;
};

struct node2 : node
{
  void accept (visitor *v) { v->visit (this); }
  char const *value;
};

struct visitor
{
  virtual ~visitor () { }
  virtual void visit (node1 *n) = 0;
  virtual void visit (node2 *n) = 0;
};

struct print_visitor : visitor
{
  virtual void visit (node1 *n) { std::cout << n->value; }
  virtual void visit (node2 *n) { std::cout << n->value; }
};

