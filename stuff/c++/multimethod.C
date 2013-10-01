struct node
{
  virtual ~node () { }
};

struct node1 : node
{
  int value;
};

struct node2 : node
{
  char const *value;
};

void print (virtual node *);
void print (static node1 *n) { std::cout << n->value; }
void print (static node2 *n) { std::cout << n->value; }
