#ifndef LISTS_H
#define LISTS_H

struct list;
struct iter;

/*@
inductive listval = | nil | cons(void*, listval);

fixpoint listval add(listval v, void* e)
{
  switch(v) {
    case nil: return cons(e, nil);
    case cons(h, t): return cons(h, add(t, e));
  }
}

fixpoint listval remove(listval v, void* e)
{
  switch(v) {
    case nil: return nil;
    case cons(h, t): return h==e?t:cons(h, remove(t, e));
  }
}

fixpoint int length(listval v)
{
  switch(v) {
    case nil: return 0;
    case cons(h, t): return 1 + length(t);
  }
}

fixpoint listval tail(listval v)
{
  switch(v) {
    case nil: return nil;
    case cons(h, t): return t;
  }
}

fixpoint void* ith(listval v, int i)
{
  switch(v) {
    case nil: return 0;
    case cons(h, t): return i==0 ? h : ith(t, i - 1);
  }
}


fixpoint bool contains(listval v, void* x)
{
  switch(v) {
    case nil: return false;
    case cons(h, t): return h==x ? true : contains(t, x);
  }
}

fixpoint bool uniqueElements(listval v)
{
  switch(v) {
    case nil: return true;
    case cons(h, t): return !contains(t, h) && uniqueElements(t);
  }
}

lemma void lengthPositive(listval v)
  requires true;
  ensures 0<=length(v);
{
  switch(v){
    case nil: return;
    case cons(h, t): lengthPositive(t);
  }
}

lemma void containsIth(listval v, int i)
  requires 0<=i &*& i < length(v);
  ensures contains(v, ith(v, i)) == true;
{
  switch(v){
    case nil: return;
    case cons(h, t):
      if(i==0){
      } else {
        containsIth(t, i - 1);
      }
  }
}

predicate foreach(listval v, predicate(void *) p)
  requires uniqueElements(v)==true &*& switch(v) { 
             case nil: return emp; 
             case cons(h, t): return p(h) &*& foreach(t, p);
           }; 

lemma void removeContains(listval v, void *x1, void *x2)
    requires !contains(v, x1);
    ensures  !contains(remove(v, x2), x1);
{
    switch (v) {
        case nil:
        case cons(h, t):
            if (h == x2) {
            } else {
                removeContains(t, x1, x2);
            }
    }
}

lemma void removeUniqueElements(listval v, void *x)
    requires uniqueElements(v) == true;
    ensures uniqueElements(remove(v, x)) == true;
{
    switch (v) {
        case nil:
        case cons(h, t):
            if (h == x) {
            } else {
                removeContains(t, h, x);
                removeUniqueElements(t, x);
            }
    }
}

lemma void remove_not_contains(listval v, struct member *mem)
  requires !contains(v, mem);
  ensures remove(v, mem) == v;
{
  switch (v) {
    case nil:
    case cons(h, t):
      if (h == mem) {
      } else {
      }
      remove_not_contains(t, mem);
  }
}

lemma void foreach_unremove(listval v, void *x)
  requires foreach(remove(v, x), ?p) &*& contains(v, x)==true &*& p(x) &*& uniqueElements(v) == true;
  ensures foreach(v, p);
{
  
  switch(v) {
    case nil: open foreach(remove(v, x), p); return;
    case cons(h, t):
      if(h==x){
        remove_not_contains(t, x);
        close foreach(v, p);
      } else {
        open foreach(remove(v, x), p);
        foreach_unremove(t, x);
        close foreach(v, p);
      }
  }
}

lemma void foreach_remove(listval v, void *x)
    requires foreach(v, ?p) &*& contains(v, x) == true;
    ensures foreach(remove(v, x), p) &*& p(x) &*& uniqueElements(v) == true;
{
    switch (v) {
        case nil:
        case cons(h, t):
            open foreach(v, p);
            if (h == x) {
            } else {
                foreach_remove(t, x);
                removeUniqueElements(v, x);
                close foreach(remove(v, x), p);
            }
    }
}



predicate list(struct list* l, listval v);

predicate iter(struct iter* i, struct list* l, listval v, int index);
@*/

struct list *create_list();
  //@ requires emp;
  //@ ensures list(result, nil);
struct iter *list_create_iter(struct list *list);
  //@ requires list(list, ?v);
  //@ ensures iter(result, list, v, 0);
bool iter_has_next(struct iter *iter);
  //@ requires iter(iter, ?l, ?v, ?i);
  //@ ensures iter(iter, l, v, i) &*& result == (i < length(v));   
void *iter_next(struct iter *iter);
  //@ requires iter(iter, ?l, ?v, ?i) &*& i < length(v);
  //@ ensures iter(iter, l, v, i + 1) &*& result==ith(v, i);
void iter_dispose(struct iter *iter);
  //@ requires iter(iter, ?l, ?v, ?i);
  //@ ensures list(l, v);
void list_add(struct list *list, void *element);
  //@ requires list(list, ?v);
  //@ ensures list(list, cons(element, v));
void list_remove(struct list *list, void *element);
  //@ requires list(list, ?v) &*& contains(v, element) == true;
  //@ ensures list(list, remove(v, element));
void list_dispose(struct list *list);
  //@ requires list(list, ?v);
  //@ ensures emp;

#endif