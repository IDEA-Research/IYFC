cp ../dag/expr.h ../out_examples/inc/
sed '10s/$/\n#include "expr.h"/' ../iyfc_include.h > ../out_examples/inc/iyfc_include.h

cp ../comm_include.h ../out_examples/inc/
cp ../err_code.h ../out_examples/inc
cp ../build/libiyfc.so ../out_examples/iyfclib
cp ../proto/*.proto ../out_examples/proto/

