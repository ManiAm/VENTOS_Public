/*_############################################################################
  _## 
  _##  collect.h  
  _##
  _##  SNMP++ v3.3
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##########################################################################*/
/*===================================================================

  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.


  SNMP++ C O L L E C T . H

  COLLECTION CLASS DEFINITION

  DESIGN + AUTHOR:  Peter E Mellquist

  DESCRIPTION: Simple Collection classes for SNMP++ classes.

=====================================================================*/
// $Id: collect.h 2359 2013-05-09 20:07:01Z fock $

#ifndef _COLLECTION_H_
#define _COLLECTION_H_

#include "snmp_pp/config_snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

#define MAXT 25     // elements per block

template <class T> class SnmpCollection
{
  class cBlock
  {
    public:
     cBlock(cBlock *p, cBlock *n) : prev(p), next(n) {};
     T *item[MAXT];
     cBlock *prev;
     cBlock *next;
  };

 public:

  /**
   * Create an empty collection.
   */
  SnmpCollection()
    : count(0), data(0,0) {};

  /**
   * Create a collection using a single template object.
   */
  SnmpCollection(const T &t)
    : count(1), data(0, 0)
  {
    data.item[0] = (T*) (t.clone());
  };

  /**
   * Create a collection with another collection (copy constructor).
   */
  SnmpCollection(const SnmpCollection<T> &c)
    : count(0), data(0, 0)
  {
    if (c.count == 0) return;

    // load up the new collection
    cBlock *current = &data;
    cBlock *nextBlock;
    int cn = 0;

    while (count < c.count)
    {
      if (cn >= MAXT)
      {
	nextBlock = new cBlock(current, 0);
	current->next = nextBlock;
	current = nextBlock;
	cn=0;
      }
      T *tmp = 0;
      c.get_element(tmp, count);
      current->item[cn] = (T*) (tmp->clone());
      count++;
      cn++;
    }
  };

  /**
   * Destroy the collection.
   */
  ~SnmpCollection()
  {
    clear();  // just delete the data
  };

  /**
   * Get the size of the collection.
   */
  int size() const
  {
    return count;
  };

  /**
   * Append an item to the collection.
   */
  SnmpCollection& operator +=(const T &i)
  {
    cBlock *current = &data;
    int cn = (int) count % MAXT;
    while (current->next)
      current = current->next;
    if ((count > 0) && ((count % MAXT) == 0))
    {
      cBlock *add = new cBlock(current, 0);
      if (!add) return *this;
      current->next = add;
      add->item[0] = (T*) (i.clone());
    }
    else
    {
      current->item[cn] = (T*) (i.clone());
    }
    count++;

    return *this;
  };

  /**
   * Assign one collection to another.
   */
  SnmpCollection &operator =(const SnmpCollection<T> &c)
  {
    if (this == &c) return *this;  // check for self assignment

    clear(); // delete the data

    if (c.count == 0) return *this;

    // load up the new collection
    cBlock *current = &data;
    cBlock *nextBlock;
    int cn = 0;
    count = 0;
    while (count < c.count)
    {
      if (cn >= MAXT)
      {
	nextBlock = new cBlock(current, 0);
	current->next = nextBlock;
	current = nextBlock;
	cn=0;
      }
      T *tmp = 0;
      c.get_element(tmp, count);
      current->item[cn] = (T*) (tmp->clone());
      count++;
      cn++;
    }

    return *this;
  };

  /**
   * Access an element in the collection.
   *
   * @return The requestet element or an empty element if out of bounds.
   */
  T operator[](const int p) const
  {
    if ((p < count) && (p >= 0))
    {
      cBlock const *current = &data;
      int bn = (int) (p / MAXT);
      int cn = (int) p % MAXT;
      for (int z=0; z<bn; z++)
	current = current->next;
      return *(current->item[cn]);
    }
    else
    {
      // return an instance of nothing!!
      T t;
      return t;
    }
  };

  /**
   * Set an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int set_element( const T& i, const int p)
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    delete current->item[cn];
    current->item[cn] = (T*) (i.clone());
    return 0;
  };

  /**
   * Get an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int get_element(T &t, const int p) const
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock const *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    t = *(current->item[cn]);
    return 0;
  };

  /**
   * Get a pointer to an element in the collection.
   *
   * @return 0 on success and -1 on failure.
   */
  int get_element(T *&t, const int p) const
  {
    if ((p < 0) || (p > count)) return -1; // not found!

    cBlock const *current = &data;
    int bn = (int) p / MAXT;
    int cn = (int) p % MAXT;
    for (int z=0; z<bn; z++)
      current = current->next;
    t = current->item[cn];
    return 0;
  };

  /**
   * Apply an function to the entire collection, iterator.
   */
  void apply(void f(T&))
  {
    T temp;
    for ( int z=0; z<count; z++)
    {
      this->get_element(temp, z);
      f(temp);
    }
  };

  /**
   * Looks for an element in the collection.
   *
   * @return TRUE if found.
   */
  int find(const T& i, int &pos) const
  {
    T temp;
    for (int z=0; z<count; z++)
    {
      this->get_element(temp, z);
      if ( temp == i) {
	pos = z;
	return true;
      }
    }
    return false;
  };

  /**
   * Delete an element in the collection.
   */
  int remove(const T& i)
  {
    // first see if we have it
    int pos;
    if (find(i, pos))
    {
      SnmpCollection<T> newCollection;

      for (int z=0; z<count; z++)
      {
	if (z != pos)
	{
	  T item;
	  get_element(item, z);
	  newCollection += item;
	}
      }

      // assign new collection to 'this'
      operator =(newCollection);

      return true;
    }
    return false;   // not found thus not removed
  };

  /**
   * Delete all elements within the collection.
   */
  void clear()
  {
    if (count == 0) return;

    cBlock *current = &data;
    int z=0;
    int cn=0;
    while ( z< count)
    {
      if (cn >= MAXT)
      {
	cn =0;
	current = current->next;
      }
      delete current->item[cn];
      cn++;
      z++;
    }

    // delete the blocks
    while (current->next)
      current = current->next;
    while (current->prev)
    {
      current = current->prev;
      delete current->next;
    }

    count = 0;
    data.next=0;
    data.prev=0;
  };

 private:
  int count;
  cBlock data;
};

#ifdef SNMP_PP_NAMESPACE
} // end of namespace Snmp_pp
#endif 

#endif  // _COLLECTION_H_

