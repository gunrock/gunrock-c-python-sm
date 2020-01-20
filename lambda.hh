#pragma once

typedef int32_t VertexId;

// No need to have this.
// The CSR representation encodes labels for edges.
struct EdgeLabelT {
  VertexId src;
  VertexId dest;
  int label;
};
typedef struct EdgeLabelT EdgeLabelT;

typedef enum {
  IntConditionT_WILDCARD = 0,
  IntConditionT_EQI = 1,
  IntConditionT_NEQI = 2,
  IntConditionT_GT = 3,
  IntConditionT_LT = 4
} IntConditionT;

typedef enum {
  StrConditionT_WILDCARD = 0,
  StrConditionT_EQ = 1,
  StrConditionT_NEQ = 2,
} StrConditionT;

// Struct that captures a condition on a Node
struct NodeConditionT {
  VertexId node;  // VertexId in the query graph
  StrConditionT cond;
  char operand[32];
};
typedef struct NodeConditionT NodeConditionT;


// Struct that captures a condition on an Edge
struct EdgeConditionT {
  VertexId src;
  VertexId dest;
  IntConditionT cond;
  int operand;
};

typedef struct EdgeConditionT EdgeConditionT;