#pragma once

#include <vector>
#include <map>


typedef uint64_t base_handle_t;
const base_handle_t INVALID_HANDLE = UINT64_MAX;

template<typename T>
class handle {
	base_handle_t key;
public:
	handle() : key(INVALID_HANDLE) {}
	explicit handle(base_handle_t key) : key(key) {}
	operator base_handle_t() { return key; }
};


class HandleGenerator
{
	base_handle_t nextHandle = 0;
public:
	base_handle_t getNextHandle() {
		return nextHandle++;
	};
};

template<typename T>
class ComponentManagerIterator
	//: public std::iterator<std::bidirectional_iterator_tag, T>
{
	typedef ComponentManagerIterator<T>  iterator;
	T* pos_;
public:
	typedef std::bidirectional_iterator_tag iterator_catagory;
	typedef T value_type;
	typedef std::ptrdiff_t difference_type;
	typedef T* pointer;
	typedef T& reference;

	ComponentManagerIterator() : pos_(nullptr) {}
	ComponentManagerIterator(T* v) : pos_(v) {}
	~ComponentManagerIterator() {}

	iterator  operator++(int) /* postfix */ { return pos_++; }
	iterator& operator++()    /* prefix */ { ++pos_; return *this; }
	iterator  operator--(int) /* postfix */ { return pos_--; }
	iterator& operator--()    /* prefix */ { --pos_; return *this; }
	T& operator* () const { return *pos_; }
	T*   operator->() const { return pos_; }
	//iterator  operator+ (ptrdiff_t v)   const { return pos_ + v; }
	bool      operator==(const iterator& rhs) const { return pos_ == rhs.pos_; }
	bool      operator!=(const iterator& rhs) const { return pos_ != rhs.pos_; }
};
#include <iostream>
// wrapper for std::vector and std::map. 
// Allows for cache-efficient iteration through the entities, as well as safe random-access through keys
// keys remain valid until their corresponding entry is deleted (Even when iterators are invalidated)
template <typename T> // T must support .getID() method and .setID() method
class ComponentManager {
	std::vector<T> components;
	std::map<base_handle_t, unsigned int> handles; // NOTE: May be changed to std::unordered_map, when average-case lookup is more important than worst-case lookup
	HandleGenerator handleGenerator;
public:
	size_t size() const {return components.size();}

	// Adds an element to the vector, and returns the key for that element
	handle<T> add(T&& component) {
		handle<T> key(handleGenerator.getNextHandle());
		component.ID = key;
		components.push_back(std::move(component));
		handles[key] = static_cast<unsigned int>(components.size() - 1);
		return key;
	} 

	T* find(handle<T> key) // Returns a pointer for temporary access to the element refered to by key. Do not store this pointer as it may be invalidated. Returns NULL if item cant be found
	{
		if (key == INVALID_HANDLE) return nullptr;

		auto index = handles.find(key);
		
		if (index == handles.end()) return nullptr;

		return &components[index->second];
	}


	void remove(handle<T> key) // may invalidate all iterators. Only invalidates the key to be deleted
	{
		if ((key != handle<T>(INVALID_HANDLE)) && handles.count(key)) {
			unsigned int index = handles[key];
			handles.erase(key); // removes the deleted elements key from the handles->index map
			
			unsigned int end_index = static_cast<unsigned int> (components.size() - 1); // gets index of last element in data

			if (index != end_index) { // check to make sure we are not deleting the last element already
				handle<T> end_key = components[end_index].ID; // gets the handle for that element

				components[index] = std::move(components[end_index]); // moves last element in data into slot of deleted element 

				handles[end_key] = index; // updates the handles->index table with the new location of that last element
			}

			components.pop_back(); // Removes the dead copy of the last element
		}
	}

	

	ComponentManagerIterator<T> begin() {
		return ComponentManagerIterator<T>(components.data());
	}
	ComponentManagerIterator<T> end() {
		return ComponentManagerIterator<T>(components.data() + components.size());
	}
};