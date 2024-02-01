/*
 *
 * MIT License
 * Copyright 2023 The IDEA Authors. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include<iostream>
#include<mutex>
#include<thread>
using namespace std;

namespace iyfc {

class Delete {
 public:
  template <typename T>
  void operator()(T* p) const {
    delete p;
  }
};
template <typename T, typename D = Delete>
class myUniqueptr {
 public:
  explicit myUniqueptr(T* pp = nullptr, const D& dd = D())
      : un_ptr(pp), del(dd) {}
  ~myUniqueptr() { del(un_ptr); }
  /* Copying and assignment are not supported   */
  myUniqueptr(const myUniqueptr&) = delete;
  myUniqueptr& operator=(const myUniqueptr&) = delete;

  /*Copy or assign a myUniqueptr (rvalue reference) that will be destroyed*/
  myUniqueptr(myUniqueptr&& right_value)
      : un_ptr(right_value.un_ptr), del(std::move(right_value.del)) {
    right_value.un_ptr = nullptr;
  }
  myUniqueptr& operator=(myUniqueptr&& right_value) noexcept {
    if (this != &right_value) {
      del(*this);
      un_ptr = right_value.un_ptr;
      del = std::move(right_value.del);
      right_value.un_ptr = nullptr;
    }
    return *this;
  }
  // u.release()   u gives up control of the pointer, returns the pointer, and sets u to null
  T* release() {
    T* tmp = un_ptr;
    un_ptr = nullptr;
    return tmp;
  }
  /*
  u.reset()   Release the object pointed to by u
  u.reset(q)  If the built-in pointer q is provided, let u point to this object
  u.reset(nullptr) Set u to empty
  */
  void reset() { del(un_ptr); }
  void reset(T* q) {
    if (un_ptr) {
      del(un_ptr);
      un_ptr = q;
    } else
      un_ptr = nullptr;
  }
  void swap(myUniqueptr& other) noexcept {
    using std::swap;
    swap(un_ptr, other.un_ptr);
    swap(del, other.del);
  }
  T* get() { return un_ptr; }
  D& get_deleter() { return del; }
  T& operator*() { return *un_ptr; }
  T* operator->() { return un_ptr; }

 private:
  T* un_ptr = nullptr;
  D del;
};



template<class T>
class mySharedPtr{
public:
	mySharedPtr(T* ptr = nullptr)
		:_pPtr(ptr)
		, _pRefCount(new int(1))
		, _pMutex(new mutex)
	{}
	~mySharedPtr()
	{
		Release();
	}
	mySharedPtr(const mySharedPtr<T>& sp)
		:_pPtr(sp._pPtr)
		, _pRefCount(sp._pRefCount)
		, _pMutex(sp._pMutex)
	{
		AddRefCount();
	}
	mySharedPtr<T>& operator=(const mySharedPtr<T>& sp)
	{
		//if (this != &sp)
		if (_pPtr != sp._pPtr)
		{
			// Release old managed resources
			Release();
			// Sharing resources that manage new objects and incrementing reference counts
			_pPtr = sp._pPtr;
			_pRefCount = sp._pRefCount;
			_pMutex = sp._pMutex;
			AddRefCount();
		}
		return *this;
	}
	T& operator*(){
		return *_pPtr;
	}
	T* operator->(){
		return _pPtr;
	}
	int UseCount() { return *_pRefCount; }
	T* Get() { return _pPtr; }
	void AddRefCount()
	{
		_pMutex->lock();
		++(*_pRefCount);
		_pMutex->unlock();
	}
private:
	void Release()
	{
		bool deleteflag = false;
		_pMutex->lock();
		if (--(*_pRefCount) == 0)
		{
			delete _pRefCount;
			delete _pPtr;
			deleteflag = true;
		}
		_pMutex->unlock();
		if (deleteflag == true)
			delete _pMutex;
	}
private:
	int *_pRefCount;
	T* _pPtr;
	mutex* _pMutex;
};

}  // namespace iyfc