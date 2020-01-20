#include "lambda.hh"

// Test 1: Select all nodes with label equal to "davis"
NodeConditionT node_eql_to_davis(VertexId nodeId) {
  NodeConditionT res;
  res.node = nodeId;
  res.cond = StrConditionT_EQ;
  strcpy(res.operand, "davis");
  return res;
}

NodeConditionT node_neq_to_berkeley(VertexId nodeId) {
  NodeConditionT res;
  res.node = nodeId;
  res.cond = StrConditionT_NEQ;
  strcpy(res.operand, "berkeley");
  return res;
}

EdgeConditionT edge_gt_n(VertexId src, VertexId dest, int n) {
  EdgeConditionT res;
  res.src = src;
  res.dest = dest;
  res.cond = IntConditionT_GT;
  res.operand = n;
  return res;
}

EdgeLabelT edgeLabel(VertexId src, VertexId dest, int label) {
  EdgeLabelT res;
  res.src = src;
  res.dest = dest;
  res.label = label;
  return res;
}

NodeConditionT dummyN() {
  NodeConditionT res;
  res.cond = StrConditionT_WILDCARD;

  return res;
}

EdgeConditionT dummyE() {
  EdgeConditionT res;
  res.cond = IntConditionT_WILDCARD;

  return res;
}
