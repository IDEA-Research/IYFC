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

#include "node_attr.h"

#include <stdexcept>

#include "util/logging.h"

using namespace std;

namespace iyfc {

#define X(name, type) name::isValid(k, v) ||
bool isValidAttr(AttrKey k, const AttrValue &v) { return IYFC_ATTR false; }
#undef X

#define X(name, type)       \
  case detail::name##Index: \
    return #name;
string getAttrName(AttrKey k) {
  switch (k) {
    IYFC_ATTR
    default:
      warn("unknown attr key");
      return {};
  }
}
#undef X

bool AttrList::has(AttrKey k) const {
  const AttrList *curr = this;
  while (true) {
    if (curr->key < k) {
      if (curr->tail) {
        curr = curr->tail.get();
      } else {
        return false;
      }
    } else {
      return curr->key == k;
    }
  }
}

/*
Find in order
*/
const AttrValue &AttrList::get(AttrKey k) const {
  const AttrList *curr = this;
  while (true) {
    if (curr->key == k) {
      return curr->value;
    } else if (curr->key < k && curr->tail) {
      curr = curr->tail.get();
    } else {
      warn("Attr not in list: %s", getAttrName(k).c_str());
      throw std::logic_error("get attr err");
    }
  }
}

/*
Arrange according to the size of the key value and insert the values after finding the corresponding position.
*/
void AttrList::set(AttrKey k, AttrValue v) {
  if (this->key == 0) {
    this->key = k;
    this->value = move(v);
  } else {
    AttrList *curr = this;
    AttrList *prev = nullptr;
    while (true) {
      if (curr->key < k) {
        if (curr->tail) {
          prev = curr;
          curr = curr->tail.get();
        } else {
          curr->tail = unique_ptr<AttrList>{new AttrList(k, move(v))};
          return;
        }
      } else if (curr->key > k) {
        if (prev) {
          auto newList = unique_ptr<AttrList>{new AttrList(k, move(v))};
          newList->tail = move(prev->tail);
          prev->tail = move(newList);
        } else {
          curr->tail = unique_ptr<AttrList>{new AttrList(move(*curr))};
          curr->key = k;
          curr->value = move(v);
        }
        return;
      } else {
        assert(curr->key == k);
        curr->value = move(v);
        return;
      }
    }
  }
}

void AttrList::assignAttrFrom(const AttrList &other) {
  if (this->key != 0) {
    this->key = 0;
    this->value = std::monostate();
    this->tail = nullptr;
  }
  if (other.key == 0) {
    return;
  }
  AttrList *lhs = this;
  const AttrList *rhs = &other;
  while (true) {
    lhs->key = rhs->key;
    lhs->value = rhs->value;
    if (rhs->tail) {
      rhs = rhs->tail.get();
      lhs->tail = std::make_unique<AttrList>();
      lhs = lhs->tail.get();
    } else {
      return;
    }
  }
}

}  // namespace iyfc
