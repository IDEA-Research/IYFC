#include "clean_util.h"

namespace iyfc {
void clearDagNode(Dag& dag) {
  auto dag_rewrite = DagTraversal(dag);
  dag_rewrite.backwardPass(CleanNodeHandler(dag, true));
}
}  // namespace iyfc