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

#include "examples.h"

using namespace std;
using namespace iyfc;

int main() {
  cout << " Help Info : " << endl;
  while (true) {
    cout << "+---------------------------------------------------------+"
         << endl;
    cout << "| The following examples should be executed while reading |"
         << endl;
    cout << "| comments in associated files in examples/.              |"
         << endl;
    cout << "+---------------------------------------------------------+"
         << endl;
    cout << "| Examples                   | Source Files               |"
         << endl;
    cout << "+----------------------------+----------------------------+"
         << endl;
    cout << "| 1. Simple CKKS             | simple_expr.cpp            |"
         << endl;
    cout << "| 2. Simple Bfv              | simple_expr.cpp            |"
         << endl;
    cout << "| 3. Psi                     | psi_bfv.cpp                |"
         << endl;
    cout << "| 4. Sort                    | sort.cpp                   |"
         << endl;
    cout << "| 5. Div                     | div.cpp                    |"
         << endl;
    cout << "| 6. Div V2                  | div_v2.cpp                 |"
         << endl;
    cout << "| 7. Serialization           | serialization.cpp          |"
         << endl;
    cout << "| 8. Query Cipher Serializa  | query.cpp                  |"
         << endl;
    cout << "| 9. Group Dag               | group.cpp                  |"
         << endl;
    cout << "| 10. Avg                    | avg.cpp                    |"
         << endl;
    cout << "| 11. Query Index plain      | query.cpp                  |"
         << endl;
    cout << "| 12. Query Cnt plain        | query.cpp                  |"
         << endl;
    cout << "| 13. Query Sum plain        | query.cpp                  |"
         << endl;
    cout << "+----------------------------+----------------------------+"
         << endl;

    int selection = 0;
    bool valid = true;
    do {
      cout << endl << "> Run example (1 ~ 13) or exit (0): ";
      if (!(cin >> selection)) {
        valid = false;
      } else if (selection < 0 || selection > 13) {
        valid = false;
      } else {
        valid = true;
      }
      if (!valid) {
        cout << "  [Beep~~] valid option: type 0 ~ 10" << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
      }
    } while (!valid);

    switch (selection) {
      case 1:
        example_simple_ckks();
        break;

      case 2:
        example_simple_bfv();
        break;

      case 3:
        example_psi_bfv();
        break;

      case 4:
        example_sort();
        break;

      case 5:
        example_div();
        break;

      case 6:
        example_div_v2();
        break;

      case 7:
        example_serialization();
        break;

      case 8:
        example_query();
        break;
      case 9:
        example_group();
        break;
      case 10:
        example_avg();
        break;
      case 11:
        example_query_index();
        break;
      case 12:
        example_query_cnt();
        break;
      case 13:
        example_query_sum();
        break;
      case 0:
        return 0;
    }
  }

  return 0;
}