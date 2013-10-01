#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>

#define MAX_PATH 255

#ifndef XMLPARSE_H
#define XMLPARSE_H

namespace xml
{
  struct attribute_pimpl
  {
    char *      name;
    bool        name_insitu;
    char *      value;
    bool        value_insitu;
  };

  enum node_type
  {
    node_null,
    node_document,
    node_element,
    node_pcdata,
    node_cdata,
    node_comment,
    node_pi,
    node_include,
    node_doctype,
    node_dtd_entity,
    node_dtd_attlist,
    node_dtd_element,
    node_dtd_notation
  };

  enum parse_flag
  {
    parse_minimal              = 1 << 0,
    parse_pi                   = 1 << 1,
    parse_doctype              = 1 << 2,
    parse_comments             = 1 << 3,
    parse_cdata                = 1 << 4,
    parse_escapes              = 1 << 5,
    parse_trim_pcdata          = 1 << 6,
    parse_trim_attribute       = 1 << 7,
    parse_trim_cdata           = 1 << 8,
    parse_trim_entity          = 1 << 9,
    parse_trim_doctype         = 1 << 10,
    parse_trim_comment         = 1 << 11,
    parse_wnorm                = 1 << 12,
    parse_dtd                  = 1 << 13,
    parse_dtd_only             = 1 << 14,
    parse_default              = (1 << 16) - 1,
    parse_noset                = 1 << 31,
  };
  
  struct node_struct
  {
    node_struct *       parent;
    char *              name;
    bool                name_insitu;
    node_type           type;
    unsigned            attributes;
    unsigned            attribute_space;
    attribute_pimpl **  attribute;
    unsigned            children;
    unsigned            child_space;
    node_struct **      child;
    char *              value;
    bool                value_insitu;
  };

  
  inline static bool
  strcatgrow (char **lhs, const char *rhs)
  {
    if (!*lhs)
      {
        *lhs = (char *) malloc (1);
        **lhs = 0;
      }

    size_t ulhs = strlen (*lhs);
    size_t urhs = strlen (rhs);
    char * temp = (char *) realloc (*lhs, ulhs + urhs + 1);
    if (!temp) return false;
    memcpy (temp + ulhs, rhs, urhs);
    temp[ulhs + urhs] = 0;
    *lhs = temp;
    return true;
  }


  inline static bool
  chartype_symbol (char c)
  {
    return (isalnum (c) || c == '_' || c == ':' || c == '-' || c == '.');
  }
  
  inline static bool
  chartype_space (char c)
  {
    return (c > 0 && c < '!');
  }

  inline static bool
  chartype_enter (char c)
  {
    return (c == '<');
  }
  
  inline static bool
  chartype_leave (char c)
  {
    return (c == '>');
  }
  
  inline static bool
  chartype_close (char c)
  {
    return (c == '/');
  }
  
  inline static bool
  chartype_equals (char c)
  {
    return (c == '=');
  }
  
  inline static bool
  chartype_special (char c)
  {
    return (c == '!');
  }
  
  inline static bool
  chartype_pi (char c)
  {
    return (c == '?');
  }
  
  inline static bool
  chartype_dash (char c)
  {
    return (c == '-');
  }
  
  inline static bool
  chartype_quote (char c)
  {
    return (c == '"' || c == '\'');
  }
  
  inline static bool
  chartype_lbracket (char c)
  {
    return (c == '[');
  }
  
  inline static bool
  chartype_rbracket (char c)
  {
    return (c == ']');
  }

  
  inline static bool
  strwtrim (char **s)
  {
    if (!s || !*s)
      return false;
    while (**s > 0 && **s < '!')
      ++*s;
    const char *temp = *s;
    while (0 != *temp++);
    long i;
    long n = temp - *s - 1;
    --n;
    for (i = n; (i > -1) && (*s) [i] > 0 && (*s) [i] < '!'; --i);
    if (i < n)
      (*s)[i + 1] = 0;
    return true;
  }

  inline static bool
  strwnorm (char **s)
  {
    if (!s || !*s)
      return false;
    while (**s > 0 && **s < '!')
      ++(*s);
    const char *temp = *s;
    while (0 != *temp++);
    long n = temp - *s - 1;
    char *norm = (char *) malloc (n + 1);
    if (!norm) return false;
    memset (norm, 0, n + 1);
    long j = 1;
    norm[0] = (*s) [0];
    long i;

    for (i = 1; i < n; ++i)
      {
        if ( (*s) [i] < '!')
          {
            if ( (*s) [i - 1] >= '!')
              {
                norm[j] = ' ';
                ++j;
              }
          }
        else
          {
            norm[j] = (*s) [i];
            ++j;
          }
      }

    if (j < n)
      {
        strncpy (*s, norm, j);
        (*s) [j] = 0;
      }

    free (norm);
    --n;
    for (i = n; (i > -1) && (*s) [i] > 0 && (*s) [i] < '!'; --i);
    if (i < n) (*s) [i + 1] = 0;
    return true;
  }

  inline static bool
  strcpyinsitu (char **dest, const char *src, bool *insitu)
  {
    if (!dest || !src || !insitu)
      return false;
    size_t l = (*dest) ? strlen (*dest) : 0;
    if (l >= strlen (src))
      {
        strcpy (*dest, src);
        return true;
      }
    else
      {
        if (*dest && !*insitu)
          free (*dest);
        *dest = NULL;
        if (strcatgrow (dest, src))
          {
            *insitu = false;
            return true;
          }
      }

    return false;
  }

  inline int
  strcmpwild_cset (const char **src, const char **dst)
  {
    int find = 0;
    int excl = 0;
    int star = 1;
    if (**src == '!')
      {
        excl = 1;
        ++(*src);
      }
    while (**src != ']' || star == 1)
      {
        if (find == 0)
          {
            if (**src == '-' && * (*src - 1) < * (*src + 1) && * (*src + 1) != ']' && star == 0)
              {
                if (**dst >= * (*src - 1) && **dst <= * (*src + 1))
                  {
                    find = 1;
                    ++(*src);
                  }
              }
            else if (**src == **dst) find = 1;
          }
        ++(*src);
        star = 0;
      }
    if (excl == 1)
      find = (1 - find);
    if (find == 1)
      ++(*dst);

    return find;
  }

  int strcmpwild_impl (const char * src, const char * dst);

  int
  strcmpwild_astr (const char ** src, const char ** dst)
  {
    int find = 1;
    ++(*src);
    while ((**dst != 0 && **src == '?') || **src == '*')
      {
        if (**src == '?')
          ++(*dst);
        ++(*src);
      }
    while (**src == '*') ++(*src);
    if (**dst == 0 && **src != 0)
      return 0;
    if (**dst == 0 && **src == 0)
      return 1;
    else
      {
        if (strcmpwild_impl (*src, *dst) == 0)
          {
            do
              {
                ++(*dst);
                while (**src != **dst && **src != '[' && **dst != 0)
                  ++(*dst);
              }
            while ( (**dst != 0) ? strcmpwild_impl (*src, *dst) == 0 : 0 != (find = 0));
          }
        if (**dst == 0 && **src == 0)
          find = 1;
        return find;
      }
  }

  int
  strcmpwild_impl (const char * src, const char * dst)
  {
    int find = 1;
    for (; *src != 0 && find == 1 && *dst != 0; ++src)
      {
        switch (*src)
          {
          case '?' : ++dst;
            break;
          case '[' : ++src;
            find = strcmpwild_cset (&src, &dst);
            break;
          case '*' : find = strcmpwild_astr (&src, &dst);
            --src;
            break;
          default :
            find = (*src == *dst);
            ++dst;
          }
      }
    while (*src == '*' && find == 1)
      ++src;
    return (find == 1 && *dst == 0 && *src == 0);
  }

  inline int strcmpwild (const char * src, const char * dst)
  {
    if (!src || !dst) return -1;
    return (strcmpwild_impl (src, dst) == 1) ? 0 : 1;
  }


  inline static attribute_pimpl* new_attribute (void)
  {
    attribute_pimpl* p = (attribute_pimpl*) malloc (sizeof (attribute_pimpl));
    if (p)
      {
        p->name = p->value = 0;
        p->name_insitu = p->value_insitu = true;
      }
    return p;
  }


  inline static node_struct *new_node (node_type type = node_element)
  {
    node_struct *p = (node_struct *) malloc (sizeof (node_struct));
    if (p)
      {
        p->name = p->value = 0;
        p->type = type;
        p->attributes = p->children = 0;
        p->name_insitu = p->value_insitu = true;
        if
        (
          type != node_document        &&
          type != node_pcdata                &&
          type != node_cdata                &&
          type != node_include        &&
          type != node_comment
        )
          p->attribute = (attribute_pimpl**) malloc (sizeof (attribute_pimpl*));
        else p->attribute = NULL;
        p->attribute_space = (p->attribute) ? 1 : 0;
        if
        (
          type == node_element ||
          type == node_doctype ||
          type == node_document
        )
          p->child = (node_struct **) malloc (sizeof (node_struct *));
        else p->child = NULL;
        p->child_space = (p->child) ? 1 : 0;
      }
    return p;
  }


  inline static node_struct *append_node (node_struct *parent, long grow, node_type type = node_element)
  {
    if (!parent) return NULL;
    if (parent->children == parent->child_space)
      {
        node_struct ** t = (node_struct **) realloc (parent->child, sizeof (node_struct *) * (parent->child_space + grow));
        if (t)
          {
            parent->child = t;
            parent->child_space += grow;
          }
      }
    node_struct *child = new_node (type);
    child->parent = parent;
    parent->child[parent->children] = child;
    parent->children++;
    return child;
  }


  inline static attribute_pimpl* append_attribute (node_struct *node_t, long grow)
  {
    if (!node_t) return NULL;
    attribute_pimpl* a = new_attribute();
    if (!a) return NULL;
    if (node_t->attributes == node_t->attribute_space)
      {
        attribute_pimpl** t = (attribute_pimpl**) realloc (node_t->attribute, sizeof (node_struct *) * (node_t->attribute_space + grow));
        if (t)
          {
            node_t->attribute = t;
            node_t->attribute_space += grow;
          }
      }
    node_t->attribute[node_t->attributes] = a;
    node_t->attributes++;
    return a;
  }


  inline static void free_node (node_struct *node_t)
  {
    if (!node_t) return;

    node_struct *cursor = node_t;


    do
      {
LOC_STEP_INTO:
        for (; cursor->children > 0; --cursor->children)
          {
            node_struct *t = cursor->child[cursor->children - 1];
            if (t && t->children)
              {
                cursor = t;
                goto LOC_STEP_INTO;
              }
            else if (t)
              {
                if (t->attributes)
                  {
                    unsigned n = t->attributes;
                    for (unsigned i = 0; i < n; ++i)
                      {
                        if (t->attribute[i]->name && !t->attribute[i]->name_insitu)
                          free (t->attribute[i]->name);
                        if (t->attribute[i]->value && !t->attribute[i]->value_insitu)
                          free (t->attribute[i]->value);
                        free (t->attribute[i]);
                      }
                  }
                if (t->attribute) free (t->attribute);
                if (t->child) free (t->child);
                if (t->name && !t->name_insitu) free (t->name);
                if (t->value && !t->value_insitu) free (t->value);
                free (t);
              }
          }
        cursor = cursor->parent;
      }
    while (cursor->children);

    if (cursor->attributes)
      {
        unsigned n = cursor->attributes;
        for (unsigned i = 0; i < n; ++i)
          {
            if (cursor->attribute[i]->name && !cursor->attribute[i]->name_insitu)
              free (cursor->attribute[i]->name);
            if (cursor->attribute[i]->value && !cursor->attribute[i]->value_insitu)
              free (cursor->attribute[i]->value);
            free (cursor->attribute[i]);
          }
      }
    if (cursor->attribute) free (cursor->attribute);
    if (cursor->child) free (cursor->child);
    if (cursor->name && !cursor->name_insitu) free (cursor->name);
    if (cursor->value && !cursor->value_insitu) free (cursor->value);
    free (cursor);
  }


  inline static void free_node_recursive (node_struct *root)
  {
    if (root)
      {
        unsigned n = root->attributes;
        unsigned i;
        for (i = 0; i < n; i++)
          {
            if (root->attribute[i]->name && !root->attribute[i]->name_insitu)
              free (root->attribute[i]->name);
            if (root->attribute[i]->value && !root->attribute[i]->value_insitu)
              free (root->attribute[i]->value);
            free (root->attribute[i]);
          }
        free (root->attribute);
        n = root->children;
        for (i = 0; i < n; i++)
          free_node_recursive (root->child[i]);
        free (root->child);
        if (root->name && !root->name_insitu) free (root->name);
        if (root->value && !root->value_insitu) free (root->value);
        free (root);
      }
  }

#define SKIPWS()                \
do {                            \
  while (chartype_space (*s))   \
    ++s;                        \
  if (*s == 0)                  \
    return s;                   \
} while (0)

#define OPTSET(OPT)     (optmsk & OPT)

#define PUSHNODE(TYPE)                          \
do {                                            \
  cursor = append_node (cursor, growby, TYPE);  \
} while (0)

#define POPNODE()               \
do {                            \
  cursor = cursor->parent;      \
} while (0)

#define SCANFOR(X)              \
do {                            \
  while (*s != 0 && !(X))       \
    ++s;                        \
  if (*s == 0)                  \
    return s;                   \
} while (0)

#define SCANWHILE(X)    \
do {                    \
  while (X)             \
    ++s;                \
  if (*s == 0)          \
    return s;           \
} while (0)

#define ENDSEG()        \
do {                    \
  ch = *s;              \
  *s = 0;               \
  ++s;                  \
  if (*s == 0)          \
    return s;           \
} while (0)

  static char * parse (char * s, node_struct *xmldoc, long growby, unsigned long optmsk = parse_default)
  {
    if (!s || !xmldoc) return s;
    char ch = 0;
    node_struct *cursor = xmldoc;
    char * mark = s;
    while (*s != 0)
      {
LOC_SEARCH:
        SCANFOR (chartype_enter (*s));
        if (chartype_enter (*s))
          {
            ++s;
LOC_CLASSIFY:
            if (chartype_pi (*s))
              {
                ++s;
                if (chartype_symbol (*s) && OPTSET (parse_pi))
                  {
                    mark = s;
                    SCANFOR (chartype_pi (*s));
                    if (chartype_pi (*s)) *s = '/';
                    s = mark;
                    PUSHNODE (node_pi);
                    goto LOC_ELEMENT;
                  }
                else
                  {
                    SCANFOR (chartype_leave (*s));
                    ++s;
                    mark = 0;
                    continue;
                  }
              }
            else if (chartype_special (*s))
              {
                ++s;
                if (chartype_dash (*s))
                  {
                    ++s;
                    if (OPTSET (parse_comments) && chartype_dash (*s))
                      {
                        ++s;
                        PUSHNODE (node_comment);
                        cursor->value = s;
                        while (*s != 0 && * (s + 1) && * (s + 2) && ! ( (chartype_dash (*s) && chartype_dash (* (s + 1))) && chartype_leave (* (s + 2)))) ++s;
                        if (*s == 0) return s;
                        *s = 0;
                        if (OPTSET (parse_trim_comment))
                          {
                            if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                            else strwtrim (&cursor->value);
                          }
                        s += 2;
                        POPNODE();
                        goto LOC_LEAVE;
                      }
                    else
                      {
                        while (*s != 0 && * (s + 1) != 0 && * (s + 2) != 0 && ! ( (chartype_dash (*s) && chartype_dash (* (s + 1))) && chartype_leave (* (s + 2)))) ++s;
                        if (*s == 0) return s;
                        s += 2;
                        goto LOC_LEAVE;
                      }
                  }
                else if (chartype_lbracket (*s))
                  {
                    ++s;
                    if (*s == 'I')
                      {
                        ++s;
                        if (*s == 'N')
                          {
                            ++s;
                            if (*s == 'C')
                              {
                                ++s;
                                if (*s == 'L')
                                  {
                                    ++s;
                                    if (*s == 'U')
                                      {
                                        ++s;
                                        if (*s == 'D')
                                          {
                                            ++s;
                                            if (*s == 'E')
                                              {
                                                ++s;
                                                if (chartype_lbracket (*s))
                                                  {
                                                    ++s;
                                                    if (OPTSET (node_cdata))
                                                      {
                                                        PUSHNODE (node_include);
                                                        cursor->value = s;
                                                        while (! (chartype_rbracket (*s) && chartype_rbracket (* (s + 1)) && chartype_leave (* (s + 2)))) ++s;
                                                        if (chartype_rbracket (*s))
                                                          {
                                                            *s = 0;
                                                            ++s;
                                                            if (OPTSET (parse_trim_cdata))
                                                              {
                                                                if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                                                else strwtrim (&cursor->value);
                                                              }
                                                          }
                                                        POPNODE();
                                                      }
                                                    else
                                                      {
                                                        while (*s != 0 && * (s + 1) != 0 && * (s + 2) != 0 && ! (chartype_rbracket (*s) && chartype_rbracket (* (s + 1)) && chartype_leave (* (s + 2)))) ++s;
                                                        ++s;
                                                      }
                                                    ++s;
                                                    goto LOC_LEAVE;
                                                  }
                                              }
                                          }
                                      }
                                  }
                              }
                          }
                      }
                    else if (*s == 'C')
                      {
                        ++s;
                        if (*s == 'D')
                          {
                            ++s;
                            if (*s == 'A')
                              {
                                ++s;
                                if (*s == 'T')
                                  {
                                    ++s;
                                    if (*s == 'A')
                                      {
                                        ++s;
                                        if (chartype_lbracket (*s))
                                          {
                                            ++s;
                                            if (OPTSET (parse_cdata))
                                              {
                                                PUSHNODE (node_cdata);
                                                cursor->value = s;
                                                while (*s != 0 && * (s + 1) != 0 && * (s + 2) != 0 && ! (chartype_rbracket (*s) && chartype_rbracket (* (s + 1)) && chartype_leave (* (s + 2)))) ++s;
                                                if (* (s + 2) == 0) return s;
                                                if (chartype_rbracket (*s))
                                                  {
                                                    *s = 0;
                                                    ++s;
                                                    if (OPTSET (parse_trim_cdata))
                                                      {
                                                        if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                                        else strwtrim (&cursor->value);
                                                      }
                                                  }
                                                POPNODE();
                                              }
                                            else
                                              {
                                                while (*s != 0 && * (s + 1) != 0 && * (s + 2) != 0 && ! (chartype_rbracket (*s) && chartype_rbracket (* (s + 1)) && chartype_leave (* (s + 2)))) ++s;
                                                ++s;
                                              }
                                            ++s;
                                            goto LOC_LEAVE;
                                          }
                                      }
                                  }
                              }
                          }
                      }
                    continue;
                  }
                else if (*s == 'D')
                  {
                    ++s;
                    if (*s == 'O')
                      {
                        ++s;
                        if (*s == 'C')
                          {
                            ++s;
                            if (*s == 'T')
                              {
                                ++s;
                                if (*s == 'Y')
                                  {
                                    ++s;
                                    if (*s == 'P')
                                      {
                                        ++s;
                                        if (*s == 'E')
                                          {
                                            ++s;
                                            SKIPWS();
                                            attribute_pimpl* a = 0;
                                            if (OPTSET (parse_doctype))
                                              {
                                                PUSHNODE (node_doctype);
                                                a = append_attribute (cursor, 3);
                                                a->value = a->name = s;
                                              }
                                            SCANWHILE (chartype_symbol (*s));
                                            ENDSEG();
                                            if (chartype_space (ch)) SKIPWS();
LOC_DOCTYPE_SYMBOL:
                                            if (chartype_symbol (*s))
                                              {
                                                mark = s;
                                                SCANWHILE (chartype_symbol (*s));
                                                if (OPTSET (parse_doctype))
                                                  {
                                                    a = append_attribute (cursor, 1);
                                                    a->value = a->name = mark;
                                                    *s = 0;
                                                  }
                                                ++s;
                                                SKIPWS();
                                              }
                                            if (chartype_quote (*s))
                                              {
LOC_DOCTYPE_QUOTE:
                                                ch = *s;
                                                ++s;
                                                mark = s;
                                                while (*s != 0 && *s != ch) ++s;
                                                if (*s != 0)
                                                  {
                                                    if (OPTSET (parse_doctype))
                                                      {
                                                        a = append_attribute (cursor, 1);
                                                        a->value = mark;
                                                        *s = 0;
                                                      }
                                                    ++s;
                                                    SKIPWS();
                                                    if (chartype_quote (*s)) goto LOC_DOCTYPE_QUOTE;
                                                    else if (chartype_symbol (*s)) goto LOC_DOCTYPE_SYMBOL;
                                                  }
                                              }
                                            if (chartype_lbracket (*s))
                                              {
                                                ++s;
                                                if (OPTSET (parse_doctype)) cursor->value = s;
                                                unsigned bd = 1;
                                                while (*s != 0)
                                                  {
                                                    if (chartype_rbracket (*s)) --bd;
                                                    else if (chartype_lbracket (*s)) ++bd;
                                                    if (bd == 0) break;
                                                    ++s;
                                                  }

                                                if (OPTSET (parse_doctype))
                                                  {
                                                    *s = 0;
                                                    if (OPTSET (parse_dtd) || OPTSET (parse_dtd_only))
                                                      {
                                                        if (OPTSET (parse_dtd))
                                                          {
                                                            parse (cursor->value, cursor, growby, optmsk);
                                                          }
                                                        if (OPTSET (parse_dtd_only)) return (s + 1);
                                                      }
                                                    else if (OPTSET (parse_trim_doctype))
                                                      {
                                                        if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                                        else strwtrim (&cursor->value);
                                                      }
                                                    ++s;
                                                    POPNODE();
                                                  }
                                                SCANFOR (chartype_leave (*s));
                                                continue;
                                              }

                                            POPNODE();
                                            continue;
                                          }
                                      }
                                  }
                              }
                          }
                      }
                  }
                else if (chartype_symbol (*s))
                  {
                    mark = s;
                    SCANWHILE (chartype_symbol (*s));
                    ENDSEG();
                    node_type e = node_dtd_entity;
                    if (strcmp (mark, "ATTLIST") == 0) e = node_dtd_attlist;
                    else if (strcmp (mark, "ELEMENT") == 0) e = node_dtd_element;
                    else if (strcmp (mark, "NOTATION") == 0) e = node_dtd_notation;
                    PUSHNODE (e);
                    if (*s != 0 && chartype_space (ch))
                      {
                        SKIPWS();
                        if (chartype_symbol (*s) || *s == '%')
                          {
                            mark = s;
                            if (*s == '%')
                              {
                                ++s;
                                if (chartype_space (*s))
                                  {
                                    SKIPWS();
                                    * (s - 1) = '%';
                                    cursor->name = (s - 1);
                                  }
                                else cursor->name = mark;
                              }
                            else cursor->name = s;
                            SCANWHILE (chartype_symbol (*s));
                            ENDSEG();
                            if (chartype_space (ch))
                              {
                                SKIPWS();
                                if (e == node_dtd_entity)
                                  {
                                    cursor->value = s;
                                    bool qq = false;
                                    while (*s != 0)
                                      {
                                        if (!qq && chartype_quote (*s))
                                          {
                                            ch = *s;
                                            qq = true;
                                          }
                                        else if (qq && *s == ch) qq = false;
                                        else if (!qq && chartype_leave (*s))
                                          {
                                            *s = 0;
                                            ++s;
                                            if (OPTSET (parse_trim_entity))
                                              {
                                                if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                                else strwtrim (&cursor->value);
                                              }
                                            POPNODE();
                                            goto LOC_SEARCH;
                                          }
                                        ++s;
                                      }
                                    if (OPTSET (parse_trim_entity))
                                      {
                                        if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                        else strwtrim (&cursor->value);
                                      }
                                  }
                                else
                                  {
                                    cursor->value = s;
                                    SCANFOR (chartype_leave (*s));
                                    *s = 0;
                                    ++s;
                                    if (OPTSET (parse_trim_entity))
                                      {
                                        if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                                        else strwtrim (&cursor->value);
                                      }
                                    POPNODE();
                                    goto LOC_SEARCH;
                                  }
                              }
                          }
                      }
                    POPNODE();
                  }
              }
            else if (chartype_symbol (*s))
              {
                cursor = append_node (cursor, growby);
LOC_ELEMENT:
                cursor->name = s;
                SCANWHILE (chartype_symbol (*s));
                ENDSEG();
                if
                (
                  *s != 0 &&
                  (
                    chartype_close (ch)
                  )
                )
                  {
                    SCANFOR (chartype_leave (*s));
                    POPNODE();
                    continue;
                  }
                else if (*s != 0 && !chartype_space (ch)) goto LOC_PCDATA;
                else if (*s != 0 && chartype_space (ch))
                  {
                    SKIPWS();
LOC_ATTRIBUTE:
                    if (chartype_symbol (*s))
                      {
                        attribute_pimpl* a = append_attribute (cursor, growby);
                        a->name = s;
                        SCANWHILE (chartype_symbol (*s));
                        ENDSEG();
                        if (*s != 0 && chartype_space (ch)) SKIPWS();
                        if (*s != 0 && (chartype_equals (ch) || chartype_equals (*s)))
                          {
                            if (chartype_equals (*s)) ++s;
                            SKIPWS();
                            if (chartype_quote (*s))
                              {
                                ch = *s;
                                ++s;
                                a->value = s;
                                SCANFOR (*s == ch);
                                ENDSEG();
                                if (OPTSET (parse_trim_attribute))
                                  {
                                    if (OPTSET (parse_wnorm)) strwnorm (&a->value);
                                    else strwtrim (&a->value);
                                  }
                                if (chartype_leave (*s))
                                  {
                                    ++s;
                                    goto LOC_PCDATA;
                                  }
                                else if (chartype_close (*s))
                                  {
                                    ++s;
                                    POPNODE();
                                    SKIPWS();
                                    if (chartype_leave (*s)) ++s;
                                    goto LOC_PCDATA;
                                  }
                                if (chartype_space (*s))
                                  {
                                    SKIPWS();
                                    goto LOC_ATTRIBUTE;
                                  }
                              }
                          }
                        if (chartype_symbol (*s)) goto LOC_ATTRIBUTE;
                        else if (*s != 0 && cursor->type == node_pi)
                          {
                            SCANFOR (chartype_close (*s));
                            SKIPWS();
                            if (chartype_close (*s)) ++s;
                            SKIPWS();
                            if (chartype_leave (*s)) ++s;
                            POPNODE();
                            goto LOC_PCDATA;
                          }
                      }
                  }
LOC_LEAVE:
                if (chartype_leave (*s))
                  {
                    ++s;
LOC_PCDATA:
                    mark = s;
                    SKIPWS();
                    if (chartype_enter (*s))
                      {
                        if (chartype_close (* (s + 1)))
                          {
                            SCANFOR (chartype_leave (*s));
                            POPNODE();
                            continue;
                          }
                        else goto LOC_SEARCH;
                      }
                    s = mark;
                    PUSHNODE (node_pcdata);
                    cursor->value = s;
                    SCANFOR (chartype_enter (*s));
                    ENDSEG();
                    if (OPTSET (parse_trim_pcdata))
                      {
                        if (OPTSET (parse_wnorm)) strwnorm (&cursor->value);
                        else strwtrim (&cursor->value);
                      }
                    POPNODE();
                    if (chartype_enter (ch))
                      {
                        if (chartype_close (*s))
                          {
                            SCANFOR (chartype_leave (*s));
                            POPNODE();
                            goto LOC_LEAVE;
                          }
                        else if (chartype_special (*s))
                          goto LOC_CLASSIFY;
                        else if (*s)
                          goto LOC_CLASSIFY;
                        else return s;
                      }
                  }

                else if (chartype_close (*s))
                  {
                    ++s;
                    if (chartype_leave (*s))
                      {
                        POPNODE();
                        ++s;
                        continue;
                      }
                  }
              }

            else if (chartype_close (*s))
              {
                SCANFOR (chartype_leave (*s));
                POPNODE();
                continue;
              }
          }
      }
    return s;
  }


  inline static bool load_file (const char * path, char ** buffer, unsigned long* size, unsigned long tempsize = 4096)
  {
    if (!path || !buffer || !size) return false;
    *size = 0;
    *buffer = 0;
    FILE *file_handle = fopen (path, "r");
    if (file_handle == NULL) return false;
    char * temp = (char *) malloc (tempsize);
    if (!temp) return false;
    unsigned long read_bytes = 0;
    memset (temp, 0, tempsize);
    while ( (read_bytes = fread (temp, 1, tempsize - 1, file_handle)) && strcatgrow (buffer, temp))
      {
        *size += read_bytes;
        memset (temp, 0, tempsize);
      }
    fclose (file_handle);
    free (temp);
    return (*size) ? true : false;
  }


  class pointer_array
    {
    protected:
      unsigned        _size;
      unsigned        _room;
      void**                        _data;
      unsigned        _grow;
    public:
      pointer_array (unsigned grow = 4) :
          _size (0),
          _room (0),
          _data (NULL),
          _grow (grow)
      {
        _data = (void**) malloc (sizeof (void*) * _grow);
        _room = (_data) ? _grow : 0;
      }
      ~pointer_array()
      {
        if (_data) free (_data);
      }
    public:
      bool empty()
      {
        return (_size == 0);
      }
      void remove_all()
      {
        _size = 0;
      }
      void clear()
      {
        if (_data)
          {
            _data = (void**) realloc (_data, sizeof (void*) * _grow);
            _room = _grow;
            _size = 0;
          }
      }
      virtual void*& operator[] (unsigned i)
      {
        static void* dummy = 0;
        if (i < _size) return _data[i];
        else return dummy;
      }
      unsigned size()
      {
        return _size;
      }
      virtual void* at (unsigned i)
      {
        if (i < _size) return _data[i];
        else return NULL;
      }
      long push_back (void* element)
      {
        if (_data)
          {
            if (_size < _room)
              {
                _data[_size] = element;
                _size++;
                return _size -1;
              }
            else
              {
                void** temp = (void**) realloc (_data, sizeof (void*) * (_size + _grow));
                if (temp)
                  {
                    _room += _grow;
                    _data = temp;
                    _data[_size] = element;
                    _size++;
                    return _size -1;
                  }
              }
          }
        return -1;
      }
    };


  class indent_stack
    {
    protected:
      char        _inch;
      char *        _stac;
      int                _size;

    public:
      indent_stack (char c = '\t') :
          _inch (c),
          _stac (0) ,
          _size (0)
      {
        _stac = (char *) malloc (1);
        *_stac = 0;
      }

      virtual ~indent_stack()
      {
        if (_stac) free (_stac);
      }

    public:
      void push()
      {
        if (_inch && _stac)
          {
            _size++;
            _stac = (char *) realloc (_stac, _size + 1);
            _stac[_size - 1] = _inch;
            _stac[_size] = 0;
          }
      }

      void pop()
      {
        if (_inch && _stac && _size > 0)
          {
            _size--;
            _stac = (char *) realloc (_stac, _size + 1);
            _stac[_size] = 0;
          }
      }


      const char * depth()
      {
        return (_inch && _stac) ? _stac : "";
      }
    };


  inline static void outer_xml (std::basic_ostream<char, std::char_traits<char> > & os, indent_stack& indent, node_struct *node_t, bool breaks = true)
  {
    if (node_t && os.good())
      {
        unsigned n, i;
        os << indent.depth();
        switch (node_t->type)
          {
          case node_dtd_attlist:
            if (node_t->name)
              {
                os << "<!ATTLIST " << node_t->name;
                if (node_t->value)
                  os << " " << node_t->value;

                os << ">";
              }
            break;
          case node_dtd_element:
            if (node_t->name)
              {
                os << "<!ELEMENT " << node_t->name;
                if (node_t->value) os << " " << node_t->value;
                os << ">";
              }
            break;
          case node_dtd_entity:
            if (node_t->name)
              {
                os << "<!ENTITY " << node_t->name;
                if (node_t->value) os << " " << node_t->value;
                os << ">";
              }
            break;
          case node_dtd_notation:
            if (node_t->name)
              {
                os << "<!NOTATION " << node_t->name;
                if (node_t->value) os << " " << node_t->value;
                os << ">";
              }
            break;
          case node_doctype:
            os << "<!DOCTYPE";
            n = node_t->attributes;
            for (i = 0; i < n; ++i)
              {
                os << " ";
                if (node_t->attribute[i]->name)
                  os << node_t->attribute[i]->name;
                else if (node_t->attribute[i]->value)
                  os << "\"" << node_t->attribute[i]->value << "\"";
              }
            if (node_t->children)
              {
                if (breaks) os << std::endl;
                else os << " ";
                os << "[";
                if (breaks) os << std::endl;
                else os << " ";
                n = node_t->children;
                indent.push();
                for (i = 0; i < n; ++i)
                  {
                    if
                    (
                      node_t->child[i] &&
                      (
                        node_t->child[i]->type == node_dtd_attlist        ||
                        node_t->child[i]->type == node_dtd_element        ||
                        node_t->child[i]->type == node_dtd_entity                ||
                        node_t->child[i]->type == node_dtd_notation
                      )
                    )
                      outer_xml (os, indent, node_t->child[i], breaks);
                  }
                indent.pop();
                os << "]";
              }
            else if (node_t->value)
              os << " [" << node_t->value << "]";
            os << ">";
            break;
          case node_pcdata:
            if (node_t->value) os << node_t->value;
            break;
          case node_cdata:
            if (node_t->value) os << "<![CDATA[" << node_t->value << "]]>";
            break;
          case node_include:
            if (node_t->value) os << "<![INCLUDE[" << node_t->value << "]]>";
            break;
          case node_comment:
            if (node_t->value) os << "<!--" << node_t->value << "-->";
            break;
          case node_element:
          case node_pi:
            os << "<";
            if (node_t->type == node_pi) os << "?";
            if (node_t->name)
              os << node_t->name;
            else os << "anonymous";
            n = node_t->attributes;
            for (i = 0; i < n; ++i)
              {
                if (node_t->attribute[i] && node_t->attribute[i]->name)
                  {
                    os << " " << node_t->attribute[i]->name;
                    if (node_t->attribute[i]->value) os << "=\"" << node_t->attribute[i]->value << "\"";
                  }
              }
            n = node_t->children;
            if (n && node_t->type == node_element)
              {
                os << ">";
                if (n == 1 && node_t->child[0]->type == node_pcdata)
                  {
                    if (node_t->child[0] && node_t->child[0]->value)
                      os << node_t->child[0]->value;
                  }
                else
                  {
                    if (breaks) os << std::endl;
                    indent.push();
                    for (i = 0; i < n; ++i) outer_xml (os, indent, node_t->child[i], breaks);
                    indent.pop();
                    os << indent.depth();
                  }
                os << "</";
                if (node_t->name) os << node_t->name;
                os << ">";
              }
            else
              {
                if (node_t->type == node_pi) os << "?>";
                else os << "/>";
              }
            break;
          default:
            break;
          }
        if (breaks) os << std::endl;
        os.flush();
      }
  }


  template <class _Ty, class _Diff, class _Pointer, class _Reference>
  class xml_iterator : public std::iterator<_Ty, _Diff, _Pointer, _Reference>
    {
    protected:
      node_struct *_vref;
      long _sscr;
    public:
      xml_iterator() : _vref (0), _sscr (-1) {}
      xml_iterator (node_struct *vref, long sscr = 0) : _vref (vref), _sscr (sscr) { }
      xml_iterator (const xml_iterator& r) : _vref (r._vref), _sscr (r._sscr) { }
      virtual ~xml_iterator() {}
    public:
      virtual bool good() = 0;
      virtual bool oob() = 0;
    public:
      virtual long subscript()
      {
        return _sscr;
      }
      virtual void subscript (long new_subscript)
      {
        _sscr = new_subscript;
      }
    public:
      virtual xml_iterator& operator= (const xml_iterator& rhs)
      {
        _vref = rhs._vref;
        _sscr = rhs._sscr;
        return *this;
      }
      virtual bool operator== (const xml_iterator& rhs)
      {
        return (_sscr == rhs._sscr);
      }
      virtual bool operator!= (const xml_iterator& rhs)
      {
        return (_sscr != rhs._sscr);
      }
      virtual bool operator< (const xml_iterator& rhs)
      {
        return (_sscr < rhs._sscr);
      }
      virtual bool operator> (const xml_iterator& rhs)
      {
        return (_sscr > rhs._sscr);
      }
      virtual bool operator<= (const xml_iterator& rhs)
      {
        return (_sscr <= rhs._sscr);
      }
      virtual bool operator>= (const xml_iterator& rhs)
      {
        return (_sscr >= rhs._sscr);
      }
      virtual xml_iterator& operator++()
      {
        _sscr++;
        return *this;
      }
      virtual xml_iterator& operator--()
      {
        _sscr--;
        return *this;
      }
      virtual _Ty& operator*() = 0;
      virtual _Ty* operator->() = 0;
    };

  class node_t;


  class xml_tree_walker
    {
    protected:
      long _deep;
    public:
      xml_tree_walker() : _deep (0) {}
      virtual ~xml_tree_walker() {}
    public:
      virtual void push()
      {
        ++_deep;
      }
      virtual void pop()
      {
        --_deep;
      }
      virtual long depth()
      {
        return (_deep > 0) ? _deep : 0;
      }
    public:
      virtual bool begin (node_t&)
      {
        return true;
      }


      virtual bool for_each (node_t&) = 0;


      virtual bool end (node_t&)
      {
        return true;
      }
    };


  class attribute_t
    {
    protected:
      attribute_pimpl* _attr;

    public:
      attribute_t() : _attr (NULL) {}
      attribute_t (attribute_pimpl* attr) : _attr (attr) {}
      attribute_t (const attribute_t& r) : _attr (r._attr) {}
      virtual ~attribute_t() {}

    public:
      void attach (attribute_pimpl* v)
      {
        _attr = v;
      }
      attribute_t& operator= (const attribute_t& r)
      {
        _attr = r._attr;
        return *this;
      }
      bool operator== (const attribute_t& r)
      {
        return (_attr == r._attr);
      }
      bool operator!= (const attribute_t& r)
      {
        return (_attr != r._attr);
      }
      operator attribute_pimpl*()
      {
        return _attr;
      }


      operator const char *()
      {
        if (empty() || !has_value()) return NULL;
        return _attr->value;
      }


      operator long()
      {
        if (empty() || !has_value()) return 0;
        return strtol (_attr->value, NULL, 10);
      }


      operator double()
      {
        if (empty() || !has_value()) return 0.0;
        return strtod (_attr->value, 0);
      }


      operator bool()
      {
        if (empty() || !has_value()) return false;
        if (* (_attr->value))
          {
            return
              (
                * (_attr->value) == '1' ||
                * (_attr->value) == 't' ||
                * (_attr->value) == 'T' ||
                * (_attr->value) == 'y' ||
                * (_attr->value) == 'Y'
              )
              ? true : false;
          }
      }


      attribute_t& operator= (const char * rhs)
      {
        if (rhs) value (rhs);
        return *this;
      }


      attribute_t& operator= (long rhs)
      {
        char temp[32] = {0};
        sprintf (temp, "%ld", rhs);
        value (temp);
        return *this;
      }


      attribute_t& operator= (double rhs)
      {
        char temp[32] = {0};
        sprintf (temp, "%lf", rhs);
        value (temp);
        return *this;
      }


      attribute_t& operator= (bool rhs)
      {
        value (rhs ? "true" : "false");
        return *this;
      }


      attribute_t& operator>> (long& rhs)
      {
        rhs = (long) * this;
        return *this;
      }


      attribute_t& operator>> (double& rhs)
      {
        rhs = (double) * this;
        return *this;
      }


      attribute_t& operator>> (bool& rhs)
      {
        rhs = (bool) * this;
        return *this;
      }


      friend long& operator<< (long& lhs, attribute_t& rhs)
      {
        lhs = (long) rhs;
        return lhs;
      }


      friend double& operator<< (double& lhs, attribute_t& rhs)
      {
        lhs = (double) rhs;
        return lhs;
      }


      friend bool& operator<< (bool& lhs, attribute_t& rhs)
      {
        lhs = (bool) rhs;
        return lhs;
      }


      friend attribute_t& operator<< (attribute_t& lhs, const long rhs)
      {
        lhs = rhs;
        return lhs;
      }


      friend attribute_t& operator<< (attribute_t& lhs, const double& rhs)
      {
        lhs = rhs;
        return lhs;
      }


      friend attribute_t& operator<< (attribute_t& lhs, const bool& rhs)
      {
        lhs = rhs;
        return lhs;
      }
    public:
      bool empty()
      {
        return (_attr == NULL);
      }
      bool has_name()
      {
        return (!empty() && _attr->name);
      }
      bool has_value()
      {
        return (!empty() && _attr->value);
      }
      bool has_name (const char * name)
      {
        return (name && !empty() && has_name() && strcmp (_attr->name, name) == 0);
      }
      bool has_value (const char * value)
      {
        return (value && !empty() && has_value() && strcmp (_attr->value, value) == 0);
      }
    public:
      const char * name()
      {
        return (!empty() && _attr->name) ? _attr->name : "";
      }
      bool name (char * new_name)
      {
        if (!empty() && new_name)
          return strcpyinsitu (&_attr->name, new_name, &_attr->name_insitu);
        return false;
      }
      const char * value()
      {
        return (!empty()) ? _attr->value : "";
      }
      bool value (const char * new_value)
      {
        if (!empty() && new_value)
          return strcpyinsitu (&_attr->value, new_value, &_attr->value_insitu);
        return false;
      }
    };


  class node_t;


  template <typename TYPE> class forward_class
    {
    protected:
      TYPE* _obj;
    public:
      forward_class() : _obj (NULL)
      {
        _obj = new TYPE();
      }
      forward_class (const TYPE& r) : _obj (NULL)
      {
        _obj = new TYPE (r);
      }
      virtual ~forward_class()
      {
        if (_obj) delete _obj;
      }
    public:
      TYPE& operator* ()
      {
        return *_obj;
      }
      TYPE* operator->()
      {
        return _obj;
      }
      operator TYPE ()
      {
        return *_obj;
      }
      operator TYPE&()
      {
        return *_obj;
      }
      operator TYPE*()
      {
        return _obj;
      }
    };


  class node_t
    {
    protected:
      node_struct *_root;
      node_struct _dummy;


    public:
      node_t() : _root (0)
      {
        memset (&_dummy, 0, sizeof (node_struct));
        _dummy.type = node_null;
        _dummy.parent = &_dummy;
        _root = &_dummy;
      }


      node_t (node_struct *p) : _root (p)
      {
        memset (&_dummy, 0, sizeof (node_struct));
      }


      node_t (const node_t& r) : _root (r._root) {}


      virtual ~node_t() {}


      node_struct *attach (node_struct *p)
      {
        node_struct *prev = _root;
        _root = p;
        return prev;
      }


    public:
    class node_iterator : public xml_iterator<node_t, long, node_t*, node_t&>
        {
        protected:
          forward_class<node_t> _wrap;
        public:
          node_iterator() : _wrap(), xml_iterator<node_t, long, node_t*, node_t&>() {}
          node_iterator (node_struct *vref, long sscr = 0) : _wrap(), xml_iterator<node_t, long, node_t*, node_t&> (vref, sscr) { }
          node_iterator (const node_iterator& r) : _wrap(), xml_iterator<node_t, long, node_t*, node_t&> (r) { }
          virtual bool good()
          {
            if
            (
              _vref != 0                        &&
              _vref->child != 0        &&
              _vref->children > 0
            )
              return true;
            return false;
          }
          virtual bool oob()
          {
            if
            (
              !good()                ||
              _sscr < 0        ||
              _sscr >= (long) _vref->children
            )
              return true;
            return false;
          }


          virtual node_t& operator*()
          {
            if (!oob()) _wrap->attach (_vref->child[_sscr]);
            else _wrap->attach (NULL);
            return (node_t&) _wrap;
          }
          virtual node_t* operator->()
          {
            if (!oob()) _wrap->attach (_vref->child[_sscr]);
            else _wrap->attach (NULL);
            return (node_t*) _wrap;
          }
        };


    class attribute_iterator : public xml_iterator<attribute_t, long, attribute_t*, attribute_t&>
        {
        protected:
          forward_class<attribute_t> _wrap;
        public:
          attribute_iterator() : _wrap(), xml_iterator<attribute_t, long, attribute_t*, attribute_t&>() {}
          attribute_iterator (node_struct *vref, long sscr = 0) : _wrap(), xml_iterator<attribute_t, long, attribute_t*, attribute_t&> (vref, sscr) { }
          attribute_iterator (const attribute_iterator& r) : _wrap(), xml_iterator<attribute_t, long, attribute_t*, attribute_t&> (r) { }
          virtual bool good()
          {
            if
            (
              _vref != 0                                &&
              _vref->attribute != 0        &&
              _vref->attributes > 0
            )
              return true;
            return false;
          }
          virtual bool oob()
          {
            if
            (
              !good()                ||
              _sscr < 0        ||
              _sscr >= (long) _vref->attributes
            )
              return true;
            return false;
          }


          virtual attribute_t& operator*()
          {
            if (!oob()) _wrap->attach (_vref->attribute[_sscr]);
            else _wrap->attach (NULL);
            return (attribute_t&) _wrap;
          }


          virtual attribute_t* operator->()
          {
            if (!oob()) _wrap->attach (_vref->attribute[_sscr]);
            else _wrap->attach (NULL);
            return (attribute_t*) _wrap;
          }
        };


      typedef node_iterator iterator;

      typedef node_iterator child_iterator;

      typedef node_iterator sibling_iterator;

      typedef attribute_iterator attribute_iterator;


      iterator begin()
      {
        return iterator (_root, 0);
      }


      iterator end()
      {
        return iterator (_root, _root->children);
      }


      iterator erase (iterator where)
      {
        remove_child ( (unsigned) where.subscript());
        return iterator (_root, 0);
      }


      child_iterator children_begin()
      {
        return child_iterator (_root, 0);
      }


      child_iterator children_end()
      {
        return child_iterator (_root, _root->children);
      }


      child_iterator children_erase (child_iterator where)
      {
        remove_child ( (unsigned) where.subscript());
        return child_iterator (_root, 0);
      }


      attribute_iterator attributes_begin()
      {
        return attribute_iterator (_root, 0);
      }


      attribute_iterator attributes_end()
      {
        return attribute_iterator (_root, _root->attributes);
      }


      attribute_iterator attributes_erase (attribute_iterator where)
      {
        remove_attribute ( (unsigned) where.subscript());
        return attribute_iterator (_root, 0);
      }


      sibling_iterator siblings_begin()
      {
        if (!empty()) return sibling_iterator (_root->parent, 0);
        return sibling_iterator();
      }


      sibling_iterator siblings_end()
      {
        if (!empty()) return sibling_iterator (_root->parent, _root->parent->children);
        return sibling_iterator();
      }


      sibling_iterator siblings_erase (sibling_iterator where)
      {
        parent().remove_child ( (unsigned) where.subscript());
        return iterator (_root->parent, 0);
      }


    public:
      operator node_struct *()
      {
        return _root;
      }
      operator void*()
      {
        return (void*) _root;
      }
      node_t& operator= (const node_t& r)
      {
        _root = r._root;
        return *this;
      }
      bool operator== (const node_t& r)
      {
        return (_root == r._root);
      }
      node_t operator[] (unsigned i)
      {
        return child (i);
      }


    public:
      bool empty()
      {
        return (_root == 0 || _root->type == node_null);
      }
      bool type_null()
      {
        return empty();
      }
      bool type_document()
      {
        return (_root && _root == _root->parent && _root->type == node_document);
      }
      bool type_element()
      {
        return (_root && _root->type == node_element);
      }
      bool type_comment()
      {
        return (_root && _root->type == node_comment);
      }
      bool type_pcdata()
      {
        return (_root && _root->type == node_pcdata);
      }
      bool type_cdata()
      {
        return (_root && _root->type == node_cdata);
      }
      bool type_include()
      {
        return (_root && _root->type == node_include);
      }
      bool type_pi()
      {
        return (_root && _root->type == node_pi);
      }
      bool type_doctype()
      {
        return (_root && _root->type == node_doctype);
      }
      bool type_dtd_item()
      {
        return (_root && _root->type > node_doctype);
      }
      bool type_dtd_attlist()
      {
        return (_root && _root->type == node_dtd_attlist);
      }
      bool type_dtd_element()
      {
        return (_root && _root->type == node_dtd_element);
      }
      bool type_dtd_entity()
      {
        return (_root && _root->type == node_dtd_entity);
      }
      bool type_dtd_notation()
      {
        return (_root && _root->type == node_dtd_notation);
      }


    public:
      bool has_value()
      {
        return (!empty() && _root->value != 0);
      }
      bool has_child_nodes()
      {
        return (!empty() && children() > 0);
      }
      bool has_attributes()
      {
        return (!empty() && attributes() > 0);
      }
      bool has_siblings()
      {
        return (!empty() && siblings() > 0);
      }
      bool has_name()
      {
        return (!empty() && _root->name != 0);
      }
      bool has_name (const char * name) const
        {
          return (name && _root && _root->name && strcmpwild (name, _root->name) == 0);
        }
      bool has_attribute (const char * name)
      {
        return (mapto_attribute_idx (name) > -1);
      }


    public:
      const char * name()
      {
        return (has_name()) ? _root->name : "";
      }
      inline bool matches_attribute_name (const char * name, const unsigned i) const
        {
          return (strcmpwild (name, _root->attribute[i]->name) == 0);
        }
      inline bool matches_child_name (const char * name, const unsigned i) const
        {
          return (strcmpwild (name, _root->child[i]->name) == 0);
        }
      inline bool matches_name (const char * name, node_struct *node_t) const
        {
          return (strcmpwild (name, node_t->name) == 0);
        }
      inline bool matches_value (const char * data, node_struct *node_t) const
        {
          return (strcmpwild (data, node_t->value) == 0);
        }
      inline bool matches_attribute_name (const char * attribute, attribute_pimpl* attr) const
        {
          return (strcmpwild (attribute, attr->name) == 0);
        }
      inline bool matches_attribute_name_value (const char * value, attribute_pimpl* attr) const
        {
          return (strcmpwild (value, attr->value) == 0);
        }
      node_type type() const
        {
          return (_root) ? (node_type) _root->type : node_null;
        }
      const char * value()
      {
        return (has_value()) ? _root->value : "";
      }
      unsigned children() const
        {
          return _root->children;
        }
      node_t child (unsigned i)
      {
        return (i < children()) ? node_t (_root->child[i]) : node_t();
      }
      unsigned attributes() const
        {
          return _root->attributes;
        }
      attribute_t attribute (unsigned i)
      {
        return (i < attributes()) ? attribute_t (_root->attribute[i]) : attribute_t();
      }


      attribute_t attribute (const char * name)
      {
        attribute_pimpl* attr = mapto_attribute_ptr (name);
        if (!attr) attr = append_attribute (name, "");
        return attribute_t (attr);
      }
      const unsigned siblings()
      {
        return (!type_document()) ? _root->parent->children : 0;
      }
      node_t sibling (unsigned i)
      {
        return (!type_document() && i < siblings()) ? node_t (_root->parent->child[i]) : node_t();
      }
      node_t parent()
      {
        return (!type_document()) ? node_t (_root->parent) : node_t();
      }


      char * child_value (char * value, const size_t valuelen) const
        {
          if (_root->children)
            {
              for (unsigned i = 0; i < _root->children; ++i)
                {
                  node_struct *node_t = _root->child[i];
                  if (node_t->value)
                    {
                      const size_t n =
                        std::min (valuelen, strlen (node_t->value));
                      strncpy (value, node_t->value, n);
                      value[n] = 0;
                      break;
                    }
                }
              return value;
            }
          return NULL;
        }


    public:
      attribute_pimpl* mapto_attribute_ptr (const char * name)
      {
        if (!_root || !name) return NULL;
        unsigned n = _root->attributes;
        for (unsigned i = 0; i < n; ++i)
          if (matches_attribute_name (name, i))
            return _root->attribute[i];
        return NULL;
      }


      int mapto_attribute_idx (const char * name)
      {
        if (!_root || !name) return -1;
        unsigned n = _root->attributes;
        for (unsigned i = 0; i < n; ++i)
          if (matches_attribute_name (name, i))
            return i;
        return -1;
      }


      node_struct *mapto_child_ptr (const char * name)
      {
        if (!_root || !name) return NULL;
        unsigned n = _root->children;
        for (unsigned i = 0; i < n; ++i)
          {
            if
            (
              _root->child[i]->name &&
              matches_child_name (name, i)
            )
              return _root->child[i];
          }
        return NULL;
      }


      int mapto_child_idx (const char * name)
      {
        if (!_root || !name) return -1;
        unsigned n = _root->children;
        for (unsigned i = 0; i < n; ++i)
          {
            if
            (
              _root->child[i]->name &&
              matches_child_name (name, i)
            )
              return i;
          }
        return -1;
      }


    public:
      void all_elements_by_name (const char * name, pointer_array& found)
      {
        if (empty() || !name) return;
        if (_root->children > 0)
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if
                (
                  _root->child[i] &&
                  _root->child[i]->name &&
                  matches_child_name (name, i)
                )
                  found.push_back (_root->child[i]);
                if (_root->child[i]->children)
                  {
                    node_t subsearch (_root->child[i]);
                    subsearch.all_elements_by_name (name, found);
                  }
              }
          }
      }


      node_t first_element_by_name (const char * name)
      {
        if (empty() || !name) return node_t();
        if (_root->children > 0)
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if
                (
                  _root->child[i]->name &&
                  matches_child_name (name, i)
                )
                  return node_t (_root->child[i]);
                else if (_root->child[i]->children)
                  {
                    node_t subsearch (_root->child[i]);
                    node_t found = subsearch.first_element_by_name (name);
                    if (!found.empty()) return found;
                  }
              }
          }
        return node_t();
      }


      node_t first_element_by_value (const char * name, const char * value)
      {
        if (empty() || !name || !value) return node_t();
        if (_root->children > 0)
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if
                (
                  _root->child[i] &&
                  _root->child[i]->name &&
                  matches_child_name (name, i)
                )
                  {
                    unsigned m = _root->child[i]->children;
                    for (unsigned j = 0; j < m; ++j)
                      {
                        if
                        (
                          _root->child[i]->child[j] &&
                          _root->child[i]->child[j]->type == node_pcdata &&
                          _root->child[i]->child[j]->value &&
                          matches_value (value, _root->child[i]->child[j])
                        )
                          return node_t (_root->child[i]);
                      }
                  }
                else if (_root->child[i] && _root->child[i]->children)
                  {
                    node_t subsearch (_root->child[i]);
                    node_t found = subsearch.first_element_by_value (name, value);
                    if (!found.empty()) return found;
                  }
              }
          }
        return node_t();
      }


      node_t first_element_by_attribute (const char * name, const char * attr_name, const char * attr_value)
      {
        if (empty() || !name || !attr_name || !attr_value) return node_t();
        if (_root->children > 0)
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if
                (
                  _root->child[i] &&
                  _root->child[i]->name &&
                  matches_name (name, _root->child[i])
                )
                  {
                    unsigned m = _root->child[i]->attributes;
                    for (unsigned j = 0; j < m; ++j)
                      {
                        if
                        (
                          _root->child[i]->attribute[j] &&
                          _root->child[i]->attribute[j]->name &&
                          matches_attribute_name (attr_name, _root->child[i]->attribute[j]) &&
                          _root->child[i]->attribute[j]->value &&
                          matches_attribute_name_value (attr_value, _root->child[i]->attribute[j])
                        )
                          return node_t (_root->child[i]);
                      }
                  }
                else if (_root->child[i] && _root->child[i]->children)
                  {
                    node_t subsearch (_root->child[i]);
                    node_t found = subsearch.first_element_by_attribute (name, attr_name, attr_value);
                    if (!found.empty()) return found;
                  }
              }
          }
        return node_t();
      }


      node_t first_node (node_type type)
      {
        if (!_root) return node_t();
        if (_root->children > 0)
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if (_root->child[i]->type == type)
                  return node_t (_root->child[i]);
                else if (_root->child[i]->children)
                  {
                    node_t subsearch (_root->child[i]);
                    node_t found = subsearch.first_node (type);
                    if (!found.empty()) return found;
                  }
              }
          }
        return node_t();
      }


      bool moveto_root()
      {
        if (empty()) return false;
        while (!type_document()) _root = _root->parent;
        return true;
      }


      bool moveto_parent()
      {
        if (empty() || type_document()) return false;
        _root = _root->parent;
        return true;
      }


      bool moveto_sibling (unsigned i)
      {
        if (empty()) return false;
        node_struct *restore = _root;
        if (moveto_parent())
          {
            if (i < children())
              {
                _root = _root->child[i];
                return true;
              }
          }
        _root = restore;
        return false;
      }


      bool moveto_first_sibling (const char * name)
      {
        if (empty() || !name) return false;
        node_struct *restore = _root;
        if (moveto_parent())
          {
            unsigned n = children();
            for (unsigned i = 0; i < n; ++i)
              {
                node_t node_t = child (i);
                if (node_t.type_element() || node_t.type_pi())
                  {
                    if (strcmpwild (name, node_t.name()) == 0)
                      {
                        _root = node_t;
                        return true;
                      }
                  }
              }
          }
        _root = restore;
        return false;
      }


      bool moveto_child (unsigned i)
      {
        if (empty()) return false;
        if (has_child_nodes() && i < children())
          {
            _root = child (i);
            return true;
          }
        return false;
      }


      bool moveto_child (const char * name)
      {
        if (empty() || !name || !has_child_nodes()) return false;
        unsigned n = children();
        for (unsigned i = 0; i < n; ++i)
          {
            node_t node_t = child (i);
            if (strcmpwild (name, node_t.name()) == 0)
              {
                _root = node_t;
                return true;
              }
          }
        return false;
      }


      bool moveto_next_sibling (const char * name)
      {
        if (empty() || type_document() || !_root->parent || !name) return false;
        unsigned n = _root->parent->children;
        for (unsigned i = 0; i < (n - 1); ++i)
          {
            if
            (
              _root->parent->child[i] &&
              _root->parent->child[i] == _root &&
              i < (n - 1)
            )
              {
                for (++i; i < n; ++i)
                  {
                    if
                    (
                      _root->parent->child[i] &&
                      _root->parent->child[i]->name &&
                      matches_name (name, _root->parent->child[i])
                    )
                      {
                        moveto_sibling (i);
                        return true;
                      }
                  }
              }
          }
        return false;
      }


      bool moveto_next_sibling()
      {
        if (empty() || type_document() || !_root->parent) return false;
        unsigned n = _root->parent->children;
        for (unsigned i = 0; i < (n - 1); ++i)
          {
            if
            (
              _root->parent->child[i] &&
              _root->parent->child[i] == _root &&
              i < (n - 1)
            )
              {
                for (++i; i < n; ++i)
                  {
                    if (_root->parent->child[i])
                      {
                        moveto_sibling (i);
                        return true;
                      }
                  }
              }
          }
        return false;
      }


      char *path (const char * delimiter = "/")
      {
        char * path = NULL;
        char * temp;
        node_t cursor = *this;
        strcatgrow (&path, cursor.name());
        while (cursor.moveto_parent() && !cursor.type_document())
          {
            temp = NULL;
            strcatgrow (&temp, cursor.name());
            strcatgrow (&temp, delimiter);
            strcatgrow (&temp, path);
            free (path);
            path = temp;
          }
        temp = NULL;
        strcatgrow (&temp, delimiter);
        strcatgrow (&temp, path);
        free (path);
        return temp;
      }


      node_t first_element_by_path (const char * path, const char * delimiter = "/")
      {
        if (!path) return node_t();
        char * temp = NULL;
        pointer_array path_segments;
        node_t found = *this;
        strcatgrow (&temp, path);
        char * name = strtok (temp, delimiter);
        while (name)
          {
            path_segments.push_back ( (void*) name);
            name = strtok (NULL, delimiter);
          }
        unsigned n = path_segments.size();
        if (n == 0) return node_t();
        if (path[0] == delimiter[0]) found.moveto_root();
        for (unsigned i = 0; i < n; ++i)
          {
            name = (char *) path_segments.at (i);
            if (name)
              {
                if (*name == '.')
                  {
                    if (strcmp (name, "..") == 0) found.moveto_parent();
                    else continue;
                  }
                else
                  {
                    unsigned j, m = found.children();
                    for (j = 0; j < m; ++j)
                      {
                        if (found.child (j).has_name (name))
                          {
                            found = found.child (j);
                            goto NEXT_ELEM;
                          }
                      }
                    if (found.moveto_next_sibling (found.name()))
                      {
                        if (i > 0) --i;
                        goto NEXT_ELEM;
                      }
                    else
                      {
                        if (!found.type_document() && found.moveto_parent() && !found.type_document())
                          {
                            if (i > 0) --i;
                            if (found.moveto_next_sibling (found.name()))
                              {
                                if (i > 0) --i;
                                goto NEXT_ELEM;
                              }
                          }
                      }
                  }
              }
NEXT_ELEM:
            ;
            if (found.type_document())
              {
                free (temp);
                return node_t();
              }
          }
        free (temp);
        return found;
      }


      bool traverse (xml_tree_walker& walker)
      {
        if (walker.depth() == 0 && !walker.begin (*this)) return false;
        if (!empty())
          {
            walker.push();
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              {
                if (_root->child[i])
                  {
                    node_t subsearch (_root->child[i]);
                    if (! (walker.for_each (subsearch) && subsearch.traverse (walker)))
                      return false;
                  }
              }
            walker.pop();
          }
        if (walker.depth() == 0 && !walker.end (*this)) return false;
        return true;
      }


    public:
      bool name (const char * new_name)
      {
        if ( (type_element() || type_pi()) && new_name)
          return strcpyinsitu (&_root->name, new_name, &_root->name_insitu);
        return false;
      }


      bool value (const char * new_value)
      {
        if ( (type_pcdata() || type_cdata() || type_comment()) && new_value)
          return strcpyinsitu (&_root->value, new_value, &_root->value_insitu);
        return false;
      }


      bool remove_attribute (unsigned i)
      {
        unsigned n = _root->attributes;
        if (i < n)
          {
            attribute_pimpl* temp = _root->attribute[i];
            --n;
            for (unsigned j = i; j < n; ++j)
              _root->attribute[j] = _root->attribute[j + 1];
            _root->attribute[n] = NULL;
            if (!temp->name_insitu) free (temp->name);
            if (!temp->value_insitu) free (temp->value);
            free (temp);
            --_root->attributes;
            return true;
          }
        return false;
      }


      bool remove_attribute (const char * name)
      {
        int i = mapto_attribute_idx (name);
        if (i > -1) return remove_attribute ( (unsigned) i);
        return false;
      }


      attribute_t append_attribute (const char * name, const char * value)
      {
        if (!name || !value) return attribute_t();
        attribute_pimpl* p = ::xml::append_attribute (_root, 1);
        if (p)
          {
            strcatgrow (&p->name, name);
            strcatgrow (&p->value, value);
            p->name_insitu = p->value_insitu = false;
            return attribute_t (p);
          }
        return attribute_t();
      }


      attribute_t append_attribute (const char * name, long value)
      {
        if (!name) return false;
        char temp[32] = {0};
        sprintf (temp, "%ld", value);
        return append_attribute (name, temp);
      }


      attribute_t append_attribute (const char * name, double value)
      {
        if (!name) return false;
        char temp[32] = {0};
        sprintf (temp, "%lf", value);
        return append_attribute (name, temp);
      }


      attribute_t append_attribute (const char * name, bool value)
      {
        if (!name) return false;
        return append_attribute (name, ( (value) ? "true" : "false"));
      }


      node_type type (node_type new_type)
      {
        node_type prev = _root->type;
        _root->type = new_type;
        return prev;
      }


      node_t append_child (node_type type)
      {
        if (type_document() || type_element())
          {
            node_struct *p = append_node (_root, 1, type);
            if (p)
              {
                p->name_insitu = p->value_insitu = false;
                return node_t (p);
              }
          }
        return node_t();
      }


      node_t insert_child (unsigned i, node_type type)
      {
        if (!type_element()) return node_t();
        unsigned n = _root->children;
        if (type_element() && i >= n) return append_child (type);
        else if (type_element() && i < n)
          {
            node_struct *p = append_node (_root, 1, type);
            if (p)
              {
                int m = (i - 1);
                for (int j = (n - 1); j > m; --j)
                  _root->child[j + 1] = _root->child[j];
                _root->child[i] = p;
                return node_t (p);
              }
          }
        return node_t();
      }


      bool remove_child (unsigned i)
      {
        unsigned n = _root->children;
        if (i < n)
          {
            node_struct *p = _root->child[i];
            --n;
            unsigned j;
            for (j = i; j < n; ++j)
              _root->child[j] = _root->child[j + 1];
            _root->child[j] = NULL;
            --_root->children;
            p->parent = p;
            free_node (p);
            return true;
          }
        return false;
      }


    public:
      void outer_xml (std::basic_ostream<char, std::char_traits<char> >& os, char indent_char = '\t', bool breaks = true)
      {
        if (empty()) return;
        indent_stack indent (indent_char);
        if (type_document())
          {
            unsigned n = _root->children;
            for (unsigned i = 0; i < n; ++i)
              ::xml::outer_xml (os, indent, _root->child[i], breaks);
          }
        else ::xml::outer_xml (os, indent, _root, breaks);
      }


      friend std::basic_ostream<char, std::char_traits<char> >& operator<< (std::basic_ostream<char, std::char_traits<char> >& os, node_t node_t)
      {
        if (!os.good()) return os;
        if ( (os.flags() | std::ostream::skipws) == std::ostream::skipws)
          node_t.outer_xml (os, 0, false);
        else node_t.outer_xml (os);
        return os;
      }
    };


  class parser_t
    {
    protected:
      node_struct *       _xmldoc;
      long                                _growby;
      bool                                _autdel;
      char *                                _buffer;
      char *                                _strpos;
      unsigned long                _optmsk;


    public:
      parser_t (unsigned long optmsk = parse_default, bool autdel = true, long growby = 4) :
          _xmldoc (0),
          _growby (growby),
          _autdel (autdel),
          _optmsk (optmsk),
          _buffer (0),
          _strpos (0)
      {
      }


      parser_t (char * xmlstr, unsigned long optmsk = parse_default, bool autdel = true, long growby = 4) :
          _xmldoc (0),
          _growby (growby),
          _autdel (autdel),
          _optmsk (optmsk),
          _buffer (0),
          _strpos (0)
      {
        parse (xmlstr, _optmsk);
      }


      virtual ~parser_t()
      {
        if (_autdel && _xmldoc) free_node (_xmldoc);
        if (_buffer) free (_buffer);
      }


    public:
      operator node_struct *()
      {
        return _xmldoc;
      }
      operator node_t()
      {
        return node_t (_xmldoc);
      }
      node_t document()
      {
        return node_t (_xmldoc);
      }


    public:
      void create()
      {
        clear();
        _xmldoc = new_node (node_document);
        _xmldoc->parent = _xmldoc;
      }


      void clear()
      {
        if (_xmldoc)
          {
            free_node (_xmldoc);
            _xmldoc = 0;
          }
        if (_buffer)
          {
            free (_buffer);
            _buffer = 0;
          }
      }


      node_struct *attach (node_struct *root)
      {
        node_struct *t = _xmldoc;
        _xmldoc = root;
        _xmldoc->parent = _xmldoc;
        return t;
      }


      node_struct *detach()
      {
        node_struct *t = _xmldoc;
        _xmldoc = 0;
        return t;
      }


      unsigned long options()
      {
        return _optmsk;
      }


      unsigned long options (unsigned long optmsk)
      {
        unsigned long prev = _optmsk;
        _optmsk = optmsk;
        return prev;
      }


      unsigned long growby()
      {
        return _growby;
      }


      unsigned long growby (long grow)
      {
        long prev = _growby;
        _growby = grow;
        return prev;
      }


      char * strpos()
      {
        return _strpos;
      }


    public:
      char *parse (char *s, unsigned long optmsk = parse_noset)
      {
        if (!s)
          return s;
        clear();
        _xmldoc = new_node (node_document);
        _xmldoc->parent = _xmldoc;
        if (optmsk != parse_noset) _optmsk = optmsk;
        return ::xml::parse (s, _xmldoc, _growby, _optmsk);
      }
      
      
      
      
      


      bool parse_file (const char * path, unsigned long optmsk = parse_noset)
      {
        if (!path) return false;
        clear();
        unsigned long bytes;
        if (optmsk != parse_noset) _optmsk = optmsk;
        if (load_file (path, &_buffer, &bytes) && bytes > 0)
          {
            _xmldoc = new_node (node_document);
            _xmldoc->parent = _xmldoc;
            char * s = ::xml::parse (_buffer, _xmldoc, _growby, _optmsk);
            _strpos = s;
            return true;
          }
        return false;
      }

    };


  class node_list: public pointer_array
    {
    public:
      node_list (unsigned grow = 4) : pointer_array (grow) { }
      virtual ~node_list() { }
    public:
      node_t at (long i)
      {
        return node_t ( (node_struct *) pointer_array::at ( (unsigned) i));
      }
      node_t operator[] (long i)
      {
        return node_t ( (node_struct *) pointer_array::at ( (unsigned) i));
      }
      friend std::ostream& operator<< (std::ostream& os, node_list& list)
      {
        if (!os.good()) return os;
        unsigned n = list.size();
        for (unsigned i = 0; i < n; ++i) os << list[i];
        return os;
      }
    };
};

#endif

int
main (int argc, char *argv[])
{
  xml::parser_t parser;

  char *data = strdup ("\
<?xml version=\"1.0\"?>\
<methodCall>\
<methodName>ermyth.login</methodName>\
<params>\
<param><value><string>pippijn</string></value></param>\
<param><value><string>password</string></value></param>\
</params>\
</methodCall>\
");

#if PROFILE
  int i = 400000;
#else
  int i = 2;
#endif
  while (--i)
    {
      if (parser.parse (data, (xml::parse_doctype
                               | xml::parse_dtd
                               | xml::parse_pi
                               | xml::parse_cdata
                               | xml::parse_comments
                               | xml::parse_wnorm
                               | xml::parse_trim_entity
                               | xml::parse_trim_attribute
                               | xml::parse_trim_pcdata
                               | xml::parse_trim_cdata
                               | xml::parse_trim_comment
                               )))
        {
#if !PROFILE
          xml::node_t document = parser.document();
          xml::node_t itelem = document.first_element_by_path ("/methodCall/params");
          if (!itelem.empty())
            {
              xml::node_t::child_iterator it = itelem.children_begin ();
              xml::node_t::child_iterator et = itelem.children_end ();
              while (it != et)
                {
                  xml::node_t param = it->first_element_by_name ("string");
                  char value[20] = { 0 };
                  param.child_value (value, 19);
                  std::cout << value << "<\n";
                  ++it;
                }
            }
          else
            std::cout << "No such element 'params'\n";
#endif
        }
      parser.clear();
    }

  free (data);

  return 0;
}
