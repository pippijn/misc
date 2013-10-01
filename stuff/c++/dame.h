/*********************************************************************
*
*  May. 24, 2006
*
*  The author disclaims copyright to this source code.
*  In place of a legal notice, here is a blessing:
*
*    May you do good and not evil.
*    May you find forgiveness for yourself and forgive others.
*    May you share freely, never taking more than you give.
*
*                                         Stolen from SQLite :-)
*  Any feedback is welcome.
*  Kohei TAKETA <k-tak@void.in>
*
*********************************************************************/

/********************************************************************
*
*  Reference
*
*   J. Aoe, K. Morimoto, and T. Sato.
*   An Efficient Implementation of Trie Structures.
*   Software Practice and Experience 22(9), 695--721, Sept. 1992.
*
*   http://citeseer.ist.psu.edu/aoe92efficient.html
*
*********************************************************************/

/********************************************************************
*
*  About thread safety
*    - It is unsafe to use the same DoubleArrayTrie object (or iterator)
*      in multiple threads at the same time.
*
*    - It is safe to use DIFFERENT DoubleArrayTrie objects
*      in differenct threads at the same time.
*
*********************************************************************/

/********************************************************************
*
*  About offsets from BASE
*
*    An offset from BASE for a character is defined as below:
*
*    offset(char c) := (unsigned char)c + 1
*
*    ex.)
*    offset('\0') == 1  (terminal symbol)
*    offset('a')  == 98
*    offset('b')  == 99
*    ...
*    An offset of zero corrupts consistency of the Double-Array,
*    thus be prohibited.
*
*********************************************************************/

/********************************************************************
*
*  About invalidation of iterators
*
*    - Any existing iterator is not invalidated by find(),
*      findPrefixedBy(), findLongestPrefixOf().
*    - All existing iterators are invalidated after insert().
*    - After erase(), all iterators which address the erased key are
*      invalidated. Other iterators remain intact.
*
*********************************************************************/

#ifndef ___DAME_H___
#define ___DAME_H___

#define DAME_VERSION 0x00010301

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cassert>

#include <list>
#include <iterator>
#include <string>
#include <utility>

#include <iostream>	// for dump
#include <vector>	// for dump
#include <algorithm>	// for dump


namespace dame {


typedef ptrdiff_t   array_index_t;
typedef size_t	    array_size_t;
typedef ptrdiff_t   array_value_t;


struct DoubleArrayElement
{
    array_index_t   base;
    array_index_t   check;
};


struct CharCodeTraits
{
    enum {
	MAX_CODE = 256
    };

    /**
     * Calculates a offset from BASE corresponding to a character.
     * @param c [in] A character whose offset is calculated.
     * @return The offset.
     */
    array_index_t offset(char c) const
    {
	return ((array_index_t)(unsigned char)c + 1);
    }

    /**
     * Calculates a character corresponding to an offset.
     * @param off [in] An offset whose corresponding character is calculated.
     * @return The character.
     */
    char character(array_index_t off) const
    {
	return (char)(unsigned char)((off-1)&0xff);
    }
};


struct DescendingCharCodeTraits
{
    enum {
	MAX_CODE = 256
    };

    array_index_t offset(char c) const
    {
	return (256 - (array_index_t)(unsigned char)c);
    }

    char character(array_index_t off) const
    {
	return (char)(unsigned char)((256-off)&0xff);
    }
};


template <typename CodeTraits>
class BasicDoubleArrayTrie
{
public:
    typedef BasicDoubleArrayTrie<CodeTraits>	trie_type;
    typedef std::string				key_type;
    typedef std::string				value_type;
    typedef array_index_t			difference_type;
    typedef std::string*			pointer;
    typedef const std::string*			const_pointer;
    typedef std::string&			reference;
    typedef const std::string&			const_reference;

public:
    enum CONSTS
    {
	BC_EXTENSION_UNIT	= CodeTraits::MAX_CODE * 2,
	TAIL_EXTENSION_UNIT	= 2048
    };

    template <typename T, typename ValueRef, bool Ascending>
    class Iterator
	: public std::iterator<std::bidirectional_iterator_tag, const std::string>
    {
    public:
	friend class BasicDoubleArrayTrie<CodeTraits>;
	typedef Iterator<T, ValueRef, Ascending> iter_type;

    protected:
	T			trie_;
	array_index_t		index_;
	bool			cached_;
	std::string		str_;

    protected:
	Iterator(T trie, array_index_t index)
	{
	    trie_ = trie;
	    index_ = index;
	    cached_ = false;
	}

	T trie() const
	{
	    return trie_;
	}

	array_index_t index() const
	{
	    return index_;
	}

    public:
	Iterator()
	{
	    trie_ = NULL;
	    index_ = 0;
	    cached_ = false;
	}

	virtual ~Iterator()
	{}

	iter_type& operator=(const iter_type& it)
	{
	    trie_ = it.trie();
	    index_ = it.index();
	    cached_ = false;
	    return *this;
	}

	bool operator==(const iter_type& it)
	{
	    return (trie_==it.trie() && index_==it.index());
	}

	bool operator!=(const iter_type& it)
	{
	    return (trie_!=it.trie() || index_!=it.index());
	}

	// move to the next key
	iter_type& operator++()
	{
	    do {
		index_ = trie_->nextNode(index_, Ascending);
	    } while (index_>0 && trie_->base(index_)>0);
	    cached_ = false;
	    return *this;
	}

	iter_type operator++(int)
	{
	    iter_type ret = *this;
	    operator++();
	    return ret;
	}

	// move to the previous key
	iter_type& operator--()
	{
	    do {
		index_ = trie_->prevNode(index_, Ascending);
	    } while (index_>0 && trie_->base(index_)>0);
	    cached_ = false;
	    return *this;
	}

	iter_type operator--(int)
	{
	    iter_type ret = *this;
	    operator--();
	    return ret;
	}

	const std::string& operator*()
	{
	    if (cached_) {
		return str_;
	    } else {
		str_ = trie_->toString(index_);
		cached_ = true;
		return str_;
	    }
	}

	const std::string* operator->()
	{
	    return &operator*();
	}

	ValueRef value() const
	{
	    return trie_->value(index_);
	}
    };

    typedef Iterator<
	trie_type*,
	array_value_t&,
	true
    > iterator;

    typedef Iterator<
	const trie_type*,
	const array_value_t&,
	true
    > const_iterator;

    typedef Iterator<
	trie_type*,
	array_value_t&,
	false
    > reverse_iterator;

    typedef Iterator<
	const trie_type*,
	const array_value_t&,
	false
    > const_reverse_iterator;

protected:
    DoubleArrayElement*		arr_;	    // BASE and CHECK
    array_size_t		arrSize_;
    char*			tail_;	    // TAIL
    array_size_t		tailSize_;
    array_size_t		tailPos_;   // POS
    array_size_t		numKeys_;
    array_value_t		defaultValue_;
    CodeTraits			traits_;

private:
    BasicDoubleArrayTrie(const trie_type&);
    BasicDoubleArrayTrie& operator=(const trie_type&);

public:
    BasicDoubleArrayTrie()
    {
	arr_ = NULL;
	arrSize_ = 0;
	tail_ = NULL;
	tailSize_ = 0;
	tailPos_ = 1;
	numKeys_ = 0;
	defaultValue_ = 0;
    }

    virtual ~BasicDoubleArrayTrie()
    {
	clear();
    }

public:
    /**
     * @return The number of keys contained in the trie.
     */
    array_size_t size() const
    {
	return numKeys_;
    }

    bool empty() const
    {
	return (numKeys_ <= 0);
    }

    /**
     * @return The default value of DoubleArrayElement::value
     */
    array_value_t defaultValue() const
    {
	return defaultValue_;
    }

    /**
     * Set the default value of DoubleArrayElement::value.
     * DoubleArrayElement::value is initialized to the default value
     * when it is newly allocated.
     */
    void setDefaultValue(array_value_t newValue)
    {
	defaultValue_ = newValue;
    }

    /**
     * Returns an iterator that addresses the first node in the trie.
     */
    iterator begin()
    {
	return ++iterator(this, 1);
    }

    /**
     * Returns a const_iterator that addresses the first node in the trie.
     */
    const_iterator begin() const
    {
	return ++const_iterator(this, 1);
    }

    /**
     * Returns an iterator that addresses the location succeeding the last
     * node in the trie.
     */
    iterator end()
    {
	return iterator(this, 0);
    }

    /**
     * Returns a const_iterator that addresses the location succeeding
     * the last node in the trie.
     */
    const_iterator end() const
    {
	return const_iterator(this, 0);
    }

    /**
     * Returns a reverse_iterator that addresses the first node in a
     * reversed trie. The iterator can be used to traverse the trie
     * in reverse order.
     */
    reverse_iterator rbegin()
    {
	return ++reverse_iterator(this, 1);
    }

    /**
     * Returns a const_reverse_iterator that addresses the first node in a
     * reversed trie. The iterator can be used to traverse the trie
     * in reverse order.
     */
    const_reverse_iterator rbegin() const
    {
	return ++const_reverse_iterator(this, 1);
    }

    /**
     * Returns a reversed_iterator that addresses the location succeeding
     * the last node in a reversed trie.
     */
    reverse_iterator rend()
    {
	return reverse_iterator(this, 0);
    }

    /**
     * Returns a const_reversed_iterator that addresses the location
     * succeeding the last node in a reversed trie.
     */
    const_reverse_iterator rend() const
    {
	return const_reverse_iterator(this, 0);
    }

    /**
     * Frees all nodes in the trie.
     */
    void clear()
    {
	if (arr_) {
	    free(arr_);
	    arr_ = NULL;
	}
	arrSize_ = 0;

	if (tail_) {
	    free(tail_);
	    tail_ = NULL;
	}
	tailSize_ = 0;
	tailPos_ = 1;
	numKeys_ = 0;
    }

    /**
     * Preallocates internal array.
     */
    void reserve(array_size_t bc, array_size_t tail)
    {
	allocateBC(bc);
	allocateTail(tail);
    }

    /**
     * Search for a key in the trie.
     * @param key [in] A string to search. Can't be NULL.
     * @return If the search succeeded, valid iterator will be returned.
     *         If the search failed, end() will be returned.
     */
    const_iterator find(const char* key) const
    {
	assert(key != NULL);

	array_index_t prevIdx=0, idx=0, bas=0, chk=0;
	size_t keyLen = strlen(key) + 1;
	if (search(key, keyLen, prevIdx, idx, bas, chk)) {
	    return const_iterator(this, idx);
	} else {
	    return end();
	}
    }

    const_iterator find(const std::string& key) const
    {
	return find(key.c_str());
    }

    iterator find(const char* key)
    {
	const_iterator it =
	    const_cast<const trie_type*>(this)->find(key);
	return iterator(this, it.index());
    }

    iterator find(const std::string& key)
    {
	return find(key.c_str());
    }

    /**
     * Retrieves a node that addresses the first node in the sub-trie,
     * that includes all keys prefixed by prefix.
     * @param prefix [in] prefix to search. Can't be NULL.
     * @param endit [out] an iterator that addresses the location succeeding
     *                    the last node in the sub-trie returned.
     * @return An iterator that addresses the first node in the sub-trie.
     */
    const_iterator findPrefixedBy(const char* prefix, const_iterator& endit) const
    {
	assert(prefix != NULL);

	array_index_t prevIdx=0, idx=0, bas=0, chk=0;
	size_t keyLen = strlen(prefix) + 1;
	search(prefix, keyLen, prevIdx, idx, bas, chk);
	if (prevIdx > 0 && toString(prevIdx) == prefix) {
	    endit = const_iterator(this, nextNodeNotPrefixed(prevIdx, true));
	    if (endit.index() > 0 && base(endit.index()) > 0) {
		++endit;
	    }
	    const_iterator ret(this, prevIdx);
	    if (prevIdx > 0 && base(prevIdx) > 0) {
		++ret;
	    }
	    return ret;
	} else {
	    endit = end();
	    return end();
	}
    }

    const_iterator findPrefixedBy(const std::string& prefix, const_iterator& endit) const
    {
	return findPrefixedBy(prefix.c_str(), endit);
    }

    iterator findPrefixedBy(const char* prefix, iterator& endit)
    {
	const_iterator tmpend;
	const_iterator it
	    = const_cast<const trie_type*>(this)->findPrefixedBy(prefix, tmpend);
	endit = iterator(this, tmpend.index());
	return iterator(this, it.index());
    }

    iterator findPrefixedBy(const std::string& prefix, iterator& endit)
    {
	return findPrefixedBy(prefix.c_str(), endit);
    }

    /**
     * Search for a key that is the longest prefix of str.
     * @param str [in] A string whose longest prefix is searched.
     *                 Can't be NULL.
     * @return An iterator that addresses the key found.
     *         If any key is found, end() is returned.
     */
    const_iterator findLongestPrefixOf(const char* str) const
    {
	assert(str != NULL);

	array_index_t prevIdx=0, idx=0, bas=0, chk=0;
	size_t keyLen = strlen(str) + 1;

	if (search(str, keyLen, prevIdx, idx, bas, chk)) {
	    // exact match
	    return const_iterator(this, idx);
	}

	// check that the rest of str starts with TAIL[-bas].
	if (chk == prevIdx && bas < 0) {
	    const char* tailstr = tail(-bas);
	    size_t len = strlen(tailstr);
	    if (!strncmp(tailstr, str+keyLen, len)) {
		return const_iterator(this, idx);
	    }
	}

	// check out ancestors
	if (prevIdx > 0) {
	    array_index_t prefixIndex;
	    do {
		// Is there a node addressing NULL character?
		prefixIndex = base(prevIdx) + traits_.offset(0);
		if (check(prefixIndex) == prevIdx) {
		    return const_iterator(this, prefixIndex);
		}
		prevIdx = parent(prevIdx);
	    } while (prevIdx);
	}

	return end();
    }

    const_iterator findLongestPrefixOf(const std::string& str) const
    {
	return findLongestPrefixOf(str.c_str());
    }

    iterator findLongestPrefixOf(const char* str)
    {
	const_iterator it
	    = const_cast<const trie_type*>(this)->findLongestPrefixOf(str);
	return iterator(this, it.index());
    }

    iterator findLongestPrefixOf(const std::string& str)
    {
	return findLongestPrefixOf(str.c_str());
    }

    /**
     * Inserts a key into the trie.
     * @param key [in] a key to be inserted. Can't be NULL.
     * @return A pair whose bool component returns true if an insertion was made
     *         and false if the trie already contained the key,
     *         and whose iterator component returns an iterator addressing
     *         the key inserted.
     * @throw bad_alloc
     */
    std::pair<iterator,bool> insert(const char* key)
    {
	assert(key != NULL);

	array_index_t prevIdx = 0;
	array_index_t idx = 0;
	array_index_t bas = 0;
	array_index_t chk = 0;
	size_t keyLen = strlen(key) + 1;
	size_t costLen = keyLen;

	if (search(key, costLen, prevIdx, idx, bas, chk)) {
	    // already exist
	    return std::pair<iterator,bool>(iterator(this, idx), false);
	}

	if (prevIdx != chk) {
	    if (chk <= 0) {
		// no collision.
		setBase(idx, -(array_index_t)tailPos_);
		setCheck(idx, prevIdx);
		if (costLen < keyLen) {
		    appendTail(key+costLen);
		} else {
		    appendTail("");
		}
		++numKeys_;
		return std::pair<iterator,bool>(iterator(this, idx), true);
	    } else {
		// collided with existing node
		// resolve collision
		resolveCollision(prevIdx, chk, key+costLen-1, keyLen-costLen+1);
		// now the collision is resolved. Try again.
		return insert(key);
	    }
	} else if (bas < 0) {
	    // collided with existing key
	    // resolve collision
	    resolveTailCollision(idx, -bas, key+costLen, keyLen-costLen);
	    // now the collision is resolved. Try again.
	    return insert(key);
	}

	// If we get here, something is wrong with the array.
	assert(!"Array inconsistency");
	return std::pair<iterator,bool>(end(), false);
    }

    std::pair<iterator,bool> insert(const std::string& key)
    {
	return insert(key.c_str());
    }

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
	for (InputIterator it=first; it!=last; ++it) {
	    insert(*it);
	}
    }

    /**
     * Erases a key.
     * @param key [in] A key to be erased. Can't be NULL.
     */
    void erase(const char* key)
    {
	erase(find(key));
    }

    void erase(const std::string& key)
    {
	erase(key.c_str());
    }

    /**
     * Erases a keyÅBDoes nothing if iter is not a key.
     * @param iter [in] An iterator addressing the key to be erased.
     */
    void erase(iterator iter)
    {
	if (iter == end())
	    return;

	freeElement(iter.index());
	--numKeys_;
    }

    // last is the position just beyond the last node removed from the trie.
    void erase(iterator first, iterator last)
    {
	while (first != last) {
	    erase(first++);
	}
    }

    /**
     * makes TAIL compact.
     * @throw bad_alloc
     */
    void shrink()
    {
	shrinkTail();
    }

protected:
    /**
     * Extends TAIL.
     * @param maxPos [in] An array index that is made accessible.
     * @throw bad_alloc
     * @note pointers based on tail_ may be invalidated after this call.
     */
    void allocateTail(array_size_t maxPos)
    {
	if (tail_ && maxPos < tailSize_) {
	    // nothing to do
	    return;
	}

	array_size_t newSize = maxPos + 1;
	newSize = (newSize / TAIL_EXTENSION_UNIT + 1) * TAIL_EXTENSION_UNIT;

	tail_ = (char*)realloc(tail_, newSize);
	if (!tail_) {
	    clear();
	    throw std::bad_alloc();
	}
	memset(tail_+tailSize_, 0, newSize-tailSize_);
	tailSize_ = newSize;
    }

    /**
     * Extends BASE and CHECK.
     * @param maxIndex [in] An array index taht is made accessible.
     * @throw bad_alloc
     * @note pointers based on arr_ may be invalidated after this call.
     */
    void allocateBC(array_size_t maxIndex)
    {
	if (arr_ && maxIndex < arrSize_) {
	    // nothing to do.
	    return;
	}

	array_size_t newSize = maxIndex + 1;
	newSize = (newSize / BC_EXTENSION_UNIT + 1) * BC_EXTENSION_UNIT;
	array_size_t numbytes = sizeof(DoubleArrayElement) * newSize;

	bool allocNew = (arr_ == NULL);
	arr_ = (DoubleArrayElement*)realloc(arr_, numbytes);
	if (!arr_) {
	    clear();
	    throw std::bad_alloc();
	}

	// initialize elements
	for (array_size_t i=arrSize_; i<newSize; ++i) {
	    arr_[i].base = 0;
	    arr_[i].check = 0;
	}

	arrSize_ = newSize;

	if (allocNew && maxIndex > 0) {
	    arr_[1].base = 1; // initial BASE[1] value
	    arr_[0].check = -1; // CHECK[0] is the root of G-link
	}

	// G-linkÇçƒç\ízÇ∑ÇÈ
	rebuildGlink();
    }

    /**
     * Rebuilds G-link.
     * G-link is a single linked list that consists of negative CHECK values.
     */
    void rebuildGlink()
    {
	if (arrSize_ <= 0)
	    return;

	// advance until CHECK[i] == -1
	array_index_t i = 0;
	array_index_t r = -(arr_[i].check);
	while (true) {
	    if (r == 1) {
		break;
	    } else {
		i = r;
		r = -(arr_[i].check);
	    }
	}

	// insert vacant elements into G-link
	array_index_t lastVacant = i;
	for (i=i+1; (array_size_t)i<arrSize_; ++i) {
	    if (arr_[i].base == 0) {
		arr_[lastVacant].check = -i;
		lastVacant = i;
	    }
	}
	arr_[lastVacant].check = -1;
    }

    /**
     * Retrieves the first vacant element in the array.
     */
    array_index_t firstVacant() const
    {
	if (arrSize_ > 0) {
	    if (arr_[0].check != -1) {
		return -(arr_[0].check);
	    } else {
		return (array_index_t)arrSize_;
	    }
	} else {
	    return 1;
	}
    }

    /**
     * Retrieves a vacant element next to index.
     * @param index an index addressing a vacant element.
     * @note index must not address an element that is in use.
     */
    array_index_t nextVacant(array_index_t index) const
    {
	assert(index > 0);

	if ((array_size_t)index >= arrSize_) {
	    return (index+1);
	} else {
	    array_index_t ret = arr_[index].check;
	    assert(ret < 0);
	    if (ret == -1) {
		return (array_index_t)arrSize_;
	    } else {
		return -ret;
	    }
	}
    }

    /**
     * Insert an element into G-link
     */
    void setVacant(array_index_t index)
    {
	assert(0<index && (array_size_t)index<arrSize_);

	if (arr_[index].check >= 0) {
	    // find the previous element of index in G-link.
	    array_index_t v = firstVacant();
	    array_index_t vprev = 0;
	    while (v < index) {
		vprev = v;
		v = nextVacant(v);
	    }

	    // rewrite links
	    arr_[index].check = arr_[vprev].check;
	    arr_[vprev].check = -index;
	}
    }

    /**
     * Erases an element from G-link
     */
    void unsetVacant(array_index_t index)
    {
	assert(0<index && (array_size_t)index<arrSize_);

	if (arr_[index].check < 0) {
	    // find the previous element of index in G-link.
	    array_index_t v = firstVacant();
	    array_index_t vprev = 0;
	    while (v < index) {
		vprev = v;
		v = nextVacant(v);
	    }
	    assert(v == index);

	    // rewrite links
	    arr_[vprev].check = arr_[index].check;
	    arr_[index].check = 0;
	}
    }

    /**
     * Clears an array element and inserts it into G-link.
     * @param index [in] an array index to be cleared.
     */
    void freeElement(array_index_t index)
    {
	assert(0<index && (array_size_t)index<arrSize_);
	arr_[index].base = 0;
	arr_[index].check = 0;
	setVacant(index);
    }

    /**
     * Ensures an array element is accessible and erases it from G-link.
     * @param index [in] an array index that is made accessible
     */
    void allocateElement(array_index_t index)
    {
	assert(index > 0);
	allocateBC(index);
	unsetVacant(index);
    }

    /**
     * Retrieves BASE[index].
     * @param index [in] An array index. Must be greater than 0.
     * @return BASE[index]
     */
    array_index_t base(array_index_t index) const
    {
	assert(index > 0);
	if ((array_size_t)index >= arrSize_) {
	    if (index == 1) {
		return 1;
	    } else {
		return 0;
	    }
	} else {
	    return arr_[index].base;
	}
    }

    /**
     * Sets a value of BASE[index].
     * @param index [in] An array index. Must be greater than 0.
     * @param value [in] A value to be set.
     * @throw bad_alloc Failed to extend arr_.
     */
    void setBase(array_index_t index, array_index_t value)
    {
	assert(index > 0);
	allocateElement(index);
	arr_[index].base = value;
    }

    /**
     * Retrieves CHECK[index].
     * @param index [in] An array index. Must be greater than 0.
     * @return CHECK[index].
     */
    array_index_t check(array_index_t index) const
    {
	assert(index > 0);
	if ((array_size_t)index >= arrSize_) {
	    return -1;
	} else {
	    return arr_[index].check;
	}
    }

    /**
     * Sets a value of CHECK[index].
     * @param index [in] An array index. Must be greater than 0.
     * @param value [in] A value to be set.
     * @throw bad_alloc Failed to extend arr_.
     */
    void setCheck(array_index_t index, array_index_t value)
    {
	assert(index > 0);
	allocateElement(index);
	arr_[index].check = value;
    }

    const array_value_t& value(array_index_t index) const
    {
	assert(0<index && (array_size_t)index<arrSize_);
	return *(array_value_t*)(tail_ - base(index));
    }

    array_value_t& value(array_index_t index)
    {
	return (
	    const_cast<array_value_t&>(
		const_cast<const trie_type*>(this)->value(index))
		);
    }

    /**
     * Retrieves a string corresponding to the specified node.
     * @param index [in] An array index.
     */
    std::string toString(array_index_t index) const
    {
	std::list<char> labels;

	array_index_t i = index;
	array_index_t j;
	char c;
	while (true) {
	    j = parent(i);
	    if (j == 0)	break;
	    if ((c = traits_.character(i-base(j)))) {
		labels.push_front(c);
	    }
	    i = j;
	}

	std::string ret(labels.begin(), labels.end());
	array_index_t bas = base(index);
	if (bas < 0) {
	    return (ret + tail(-bas));
	} else {
	    return ret;
	}
    }

    /**
     * Retrieves TAIL[index].
     * @param index [in] An TAIL index. Must be greater than 0,
     *                   and less than tailPos_ + sizeof(array_value_t).
     */
    char* tail(array_index_t index) const
    {
	assert(0<index && (array_size_t)index+sizeof(array_value_t)<tailPos_);
	return tail_ + index + sizeof(array_value_t);
    }

    /**
     * Appends a new string to TAIL.
     * @param str [in] A string to be added to TAIL.
     * @note Pointers based on tail_ may be invalidated after this call.
     */
    void appendTail(const char* str)
    {
	size_t len = strlen(str) + 1;
	array_size_t newPos = tailPos_ + len + sizeof(array_value_t);
	allocateTail(newPos-1);
	*(array_value_t*)(tail_+tailPos_) = defaultValue_;
	memcpy(tail_+tailPos_+sizeof(array_value_t), str, len);
	tailPos_ = newPos;
    }

    /**
     * Truncates a string in TAIL.
     * @param index [in] A TAIL index.
     * @param newstr [in] A truncated string. Must be shorter than
     *                    the original string.
     */
    void truncTail(array_index_t index, const char* newstr)
    {
	char* dst = tail(index);
	size_t newlen = strlen(newstr);
	assert(strlen(dst) >= newlen);
	memmove(dst, newstr, newlen+1);
    }

    /**
     * Compares str with a string in TAIL.
     * @param index [in] A TAIL index.
     * @param str [in] A stirng to be compared with.
     * @return Returns true if two strings are identical.
     *         Else, returns false.
     */
    bool compareTail(array_index_t index, const char* str) const
    {
	assert(0<index && (array_size_t)index+sizeof(array_value_t)<tailPos_);
	return !strcmp(str, tail_+index+sizeof(array_value_t));
    }

    /**
     * compactify TAIL.
     * @throw bad_alloc Failed to allocate new TAIL.
     */
    void shrinkTail()
    {
	if (tailSize_ <= 0)
	    return;

	// Calculate needed length
	array_size_t totalLen = 0;
	for (array_size_t i=1; i<arrSize_; ++i) {
	    if (base(i) < 0) {
		totalLen += sizeof(array_value_t) + strlen(tail(-base(i))) + 1;
	    }
	}

	// allocate new TAIL
	char* newTail = (char*)malloc(totalLen+1);
	if (!newTail)
	    throw std::bad_alloc();

	// copy all strings in the current TAIL to the new TAIL.
	array_size_t pos = 1;
	for (array_size_t i=1; i<arrSize_; ++i) {
	    if (base(i) < 0) {
		size_t len = strlen(tail(-base(i))) + 1;
		*(array_value_t*)(newTail+pos) = value(i);
		memcpy(newTail+pos+sizeof(array_value_t), tail(-base(i)), len);
		setBase(i, -(array_index_t)pos);
		pos += len + sizeof(array_value_t);
	    }
	}
	//std::cout << "Compactified from " << tailSize_ << " to " << totalLen << std::endl;

	free(tail_);
	tail_ = newTail;
	tailSize_ = totalLen+1;
	tailPos_ = totalLen+1;
    }

    /**
     * Move a node to another position in the array.
     * @param srcIndex [in] Source
     * @param dstIndex [in] Destination. Must be vacant.
     * @throw bad_alloc Failed to extend the array.
     */
    void moveElement(array_index_t srcIndex, array_index_t dstIndex)
    {
	assert(srcIndex > 0 && dstIndex > 0);

	if (srcIndex == dstIndex)
	    return;

	// copy element
	array_index_t bas = base(srcIndex);
	setBase(dstIndex, bas);
	setCheck(dstIndex, check(srcIndex));

	// fix children's CHECK
	if (bas > 0) {
	    array_index_t arcs[CodeTraits::MAX_CODE];
	    array_index_t numArcs = leavingArcs(srcIndex, arcs);
	    for (array_index_t i=0; i<numArcs; ++i) {
		setCheck(bas+arcs[i], dstIndex);
	    }
	}

	// clear old position
	freeElement(srcIndex);
    }

    /**
     * Search for a node that can have the specified arc(s).
     * @param list [in] Arc labels.
     * @param num [in] The number of labels stored in list.
     * @return The array index of the node.
     */
    array_index_t xCheck(const array_index_t* list, size_t num) const
    {
	assert(list && num > 0);

	array_index_t v = firstVacant();
	array_index_t ret = 0;
	bool found = false;

	do {
	    found = true;
	    ret = v - list[0];
	    v = nextVacant(v);
	    if (ret > 0) {
		for (size_t i=1; i<num; ++i) {
		    if (check(ret + list[i]) >= 0) {
			found = false;
			break;
		    }
		}
	    } else {
		found = false;
	    }
	} while (!found);

	return ret;
    }

    /**
     * Search for a node that can have the specified arc.
     * @param c [in] An arc label (= offset from BASE)
     * @return The array index of the node.
     */
    array_index_t xCheck(array_index_t c) const
    {
	array_index_t v = firstVacant();
	while (true) {
	    if (v > c) break;
	    v = nextVacant(v);
	}

	return (v - c);
    }

    /**
     * Stores all labels of arcs leaving the specified node in list,
     * and returns the number of stored arcs.
     * @param index [in] An array index
     * @param list [out] A buffer in which the method stores offsets.
     *                   The size of list must be equal to or greater
     *                   than MAX_CODE.
     * @return The number of stored arcs.
     */
    array_index_t leavingArcs(array_index_t index, array_index_t* list) const
    {
	array_index_t numChildren = 0;
	array_index_t* p = list;
	array_index_t b = base(index);
	if (b > 0) {
	    for (array_index_t i=1; i<=(array_index_t)CodeTraits::MAX_CODE; ++i) {
		if (check(b+i) == index) {
		    *p++ = i;
		    ++numChildren;
		}
	    }
	}
	return numChildren;
    }

    /**
     * Search for a key in the trie.
     * @param key [in] A key to be searched
     * @param keyLen [in/out] Reference to a variable that stores the length of
     *               key including last NULL. When the search ends, the method
     *               sets the number of consumed characters to the variable.
     * @param prevIdx [out] An index of parent node of idx is returned.
     * @param idx [out] An index of node that caused the search end is returned.
     * @param bas [out] BASE[idx] is returned.
     * @param chk [out] CHECK[idx] is returned.
     * @return If the key was found, returns true. Else, returns false.
     */
    bool search(
	const char*	key,
	size_t&		keyLen,
	array_index_t&	prevIdx,
	array_index_t&	idx,
	array_index_t&	bas,
	array_index_t&	chk) const
    {
	array_index_t curIndex = 0;
	array_index_t nextIndex = 1;
	array_index_t nextBase = base(1);
	array_index_t nextCheck = 0;
	bool found = false;

	size_t pos = 0;
	while (pos < keyLen) {
	    curIndex = nextIndex;
	    nextIndex = nextBase + traits_.offset(key[pos]);
	    nextBase = base(nextIndex);
	    nextCheck = check(nextIndex);
	    ++pos;

	    if (nextCheck != curIndex) {
		// collision detected.
		break;
	    } else if (nextBase < 0) {
		// compare the rest of key and TAIL
		assert((array_size_t)(-nextBase) < tailSize_);
		if (pos >= keyLen) {
		    found = compareTail(-nextBase, "");
		} else {
		    found = compareTail(-nextBase, key+pos);
		}
		break;
	    }
	}

	keyLen = pos;
	prevIdx = curIndex;
	idx = nextIndex;
	bas = nextBase;
	chk = nextCheck;

	return found;
    }

    // resolve collision with existing node
    void resolveCollision(
	array_index_t curIndex,	    // current node
	array_index_t anotherIndex, // node that is already using the array element
	const char* key,	    // pointer addressing the character which caused the collision
	size_t keyLen)		    // length of key. including last NULL.
    {
	array_index_t curArcs[CodeTraits::MAX_CODE];
	array_index_t anotherArcs[CodeTraits::MAX_CODE];
	array_index_t numCurArcs = leavingArcs(curIndex, curArcs);
	array_index_t numAnotherArcs = leavingArcs(anotherIndex, anotherArcs);
	// we must add an arc labeled key[0] to curIndex.
	curArcs[numCurArcs++] = traits_.offset(key[0]);

	// change the node which has less arcs.
	if (numCurArcs < numAnotherArcs) {
	    // change the current node.
	    array_index_t oldBase = base(curIndex);
	    array_index_t newBase = xCheck(curArcs, numCurArcs);
	    setBase(curIndex, newBase);
	    // move children
	    // last arc (key[0]) does not exist yet, so we don't move it.
	    for (array_index_t i=0; i<numCurArcs-1; ++i) {
		moveElement(oldBase + curArcs[i], newBase + curArcs[i]);
	    }
	} else {
	    // change the other node.
	    array_index_t oldBase = base(anotherIndex);
	    array_index_t newBase = xCheck(anotherArcs, numAnotherArcs);
	    setBase(anotherIndex, newBase);
	    // move children
	    for (array_index_t i=0; i<numAnotherArcs; ++i) {
		moveElement(oldBase + anotherArcs[i], newBase + anotherArcs[i]);
	    }
	}
    }

    // resolve collision with existing key
    void resolveTailCollision(
	array_index_t index,	    // node to be inserted
	array_index_t tailIndex,    // index of TAIL array
	const char* key,	    // pointer addressing the character which caused the collision
	size_t keyLen)		    // length of key. including last NULL.
    {
	array_index_t newIndex = 0;
	array_index_t newBase = 0;

	char* singleStr = tail(tailIndex);
	size_t singleStrLen = strlen(singleStr) + 1; // includes NULL

	// insert common prefix of key and singleStr.
	size_t pos = 0;
	for (; pos<keyLen && pos<singleStrLen; ++pos) {
	    if (key[pos] != singleStr[pos])
		break;

	    array_index_t c = traits_.offset(key[pos]);
	    newBase = xCheck(c);
	    newIndex = newBase + c;
	    setBase(index, newBase);
	    setCheck(newIndex, index);
	    index = newIndex;
	}
	assert(pos<keyLen && pos<singleStrLen);

	// now key[pos] != singleStr[pos]
	array_index_t list[2];
	list[0] = traits_.offset(key[pos]);
	list[1] = traits_.offset(singleStr[pos]);
	newBase = xCheck(list, 2);
	setBase(index, newBase);

	// rellocate singleStr
	newIndex = newBase + traits_.offset(singleStr[pos]);
	setBase(newIndex, -tailIndex);
	setCheck(newIndex, index);
	if (pos+1 < singleStrLen) {
	    truncTail(tailIndex, singleStr+pos+1);
	} else {
	    truncTail(tailIndex, "");
	}
    }

protected:
    /////////////////////////////////////////////////////////////////////
    //  ITERATION
    /////////////////////////////////////////////////////////////////////
    // Retrieves the parent node of the specified node.
    // Returns 0 if not found.
    array_index_t parent(array_index_t index) const
    {
	array_index_t ret = check(index);
	if (ret > 0) {
	    return ret;
	} else {
	    return 0;
	}
    }

    // Retrieves the first/last child node of the specified node.
    // Returns 0 if not found.
    array_index_t child(array_index_t index, bool first) const
    {
	array_index_t b = base(index);
	if (b > 0) {
	    if (first) {
		for (unsigned int i=1; i<=(unsigned int)CodeTraits::MAX_CODE; ++i) {
		    if (check(b+i) == index)
			return (b+i);
		}
	    } else {
		for (unsigned int i=(unsigned int)CodeTraits::MAX_CODE; i>0; --i) {
		    if (check(b+i) == index)
			return (b+i);
		}
	    }
	}

	return 0;
    }

    // Retrieves the next sibling node of the specified node.
    // Returns 0 if not found.
    array_index_t sibling(array_index_t index, bool younger) const
    {
	array_index_t parentidx = check(index);
	if (parentidx > 0) {
	    array_index_t b = base(parentidx);
	    if (younger) {
		for (array_index_t i=(index-b+1); i<=(array_index_t)CodeTraits::MAX_CODE; ++i) {
		    if (check(b+i) == parentidx)
			return (b+i);
		}
	    } else {
		for (array_index_t i=(index-b-1); i>0; --i) {
		    if (check(b+i) == parentidx)
			return (b+i);
		}
	    }
	}

	return 0;
    }

    // Retrieves the next node of the specified node. Returns 0 if not found.
    // If asc==true, follow arcs in ascending order.
    // If asc==false, follow arcs in descending order.
    array_index_t nextNode(array_index_t index, bool asc) const
    {
	array_index_t tmp;
	if ((tmp=child(index, asc))) {
	    return tmp;
	} else {
	    return nextNodeNotPrefixed(index, asc);
	}
    }

    array_index_t nextNodeNotPrefixed(array_index_t index, bool asc) const
    {
	array_index_t ret = 0;
	array_index_t tmp;

	ret = index;
	while (ret) {
	    if ((tmp=sibling(ret, asc))) {
		ret = tmp;
		break;
	    }
	    ret = parent(ret);
	}

	return ret;
    }

    // Retrieves the previous node of the specified node.
    // Returns 0 if not found.
    // If asc==true, follow arcs in ascending order.
    // If asc==false, follow arcs in descending order.
    array_index_t prevNode(array_index_t index, bool asc) const
    {
	array_index_t ret = 0;
	array_index_t tmp, tmp2;

	if ((tmp=sibling(index, !asc))) {
	    tmp2 = child(tmp, !asc);
	    while (tmp2) {
		tmp = tmp2;
		tmp2 = child(tmp2, !asc);
	    }
	    ret = tmp;
	} else {
	    ret = parent(index);
	}

	return ret;
    }

public:
    /////////////////////////////////////////////////////////////////////
    //  Debug
    /////////////////////////////////////////////////////////////////////
    void dump() const
    {
	std::vector<array_index_t> tails;

	// dump BC
	std::cout << "IDX\tBASE\tCHECK" << std::endl;
	for (array_index_t i=0; i<(array_index_t)arrSize_; ++i) {
	    std::cout
		<< (long long)i	<< "\t"
		<< (long long)arr_[i].base << "\t"
		<< (long long)arr_[i].check << std::endl;
	    if (arr_[i].base < 0) {
		tails.push_back(-arr_[i].base);
	    }
	}
	// dump TAIL
	std::sort(tails.begin(), tails.end());
	for (size_t i=0; i<tails.size(); ++i) {
	    std::cout
		<< "TAIL[" << (long long)tails[i] << "] = "
		<< "(" << (long long)*(array_value_t*)(tail_+tails[i]) << ") "
		<< (tail_+tails[i]+sizeof(array_value_t)) << std::endl;
	}
    }
};


typedef BasicDoubleArrayTrie<CharCodeTraits> DoubleArrayTrie;
typedef BasicDoubleArrayTrie<DescendingCharCodeTraits> DescendingDoubleArrayTrie;


};

#endif
