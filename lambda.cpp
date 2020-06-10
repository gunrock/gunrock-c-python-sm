#include <cstddef>
#include <vector>
#include <algorithm>
#include <iostream>
#include "lambda.hh"
#include "test.cpp"
#include "utils.cpp"



#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>

using namespace boost;

class WebPage
{
 public:
  std::string url;
};


using namespace std;

/**
 * Lookup condition for a query node
 */

NodeConditionT getQueryNodeConditionById(
    const vector<NodeConditionT> query_nodeConditions, int query_nodeId) {
  auto it =
      std::find_if(query_nodeConditions.begin(), query_nodeConditions.end(),
                   [query_nodeId](const NodeConditionT &cond) {
                     return cond.node == query_nodeId;
                   });
  if (it != query_nodeConditions.end()) {
    return *it;
  } else {
    assert(false);
  }
}

/**
 * Lookup condition for a query edge
 */

EdgeConditionT getQueryEdgeConditionById(
    const vector<EdgeConditionT> query_edgeConditions, VertexId qsrc,
    VertexId qdest) {
  auto it =
      std::find_if(query_edgeConditions.begin(), query_edgeConditions.end(),
                   [qsrc, qdest](const EdgeConditionT &cond) {
                     return cond.src == qsrc && cond.dest == qdest;
                   });
  if (it != query_edgeConditions.end()) {
    return *it;
  } else {
    assert(false);
  }
}

/**
 * Lookup metadata for a base node
 */

const char *getBaseNodeMetadataById(const char *const base_nodeLabels[],
                                    VertexId base_nodeId) {
  // Since we do not want to use a hashmap in C-land,
  // we pass vertex attributes as a list of VLabelTs.
  // [ ] We must make sure that VertexId matches the index of the vertex in the
  // CSR representation.
  return base_nodeLabels[base_nodeId];
}

/**
 * Lookup metadata for a base edge
 */

EdgeLabelT getBaseEdgeMetadataById(vector<EdgeLabelT> base_edgeLabels,
                                   VertexId src, VertexId dest) {
  // Since we do not want to use a hashmap in C-land,
  // we pass vertex attributes as a list of VLabelTs.
  // [ ] We must make sure that VertexId matches the index of the vertex in the
  // CSR representation.
  auto it = std::find_if(base_edgeLabels.begin(), base_edgeLabels.end(),
                         [src, dest](const EdgeLabelT &label) {
                           return label.src == src && label.dest == dest;
                         });
  if (it != base_edgeLabels.end()) {
    return *it;
  } else {
    assert(false);
  }
}

/**
 * Check to see if we can match a query node with a base node
 * To do this, we
 *   (1) lookup the condition for the query node
 *   (2) lookup the metadata for the base node
 *   (3) evalaute the condition by testing it on the metadata
 */

// String comparison operators
std::function<bool(VertexId, VertexId)> nodeLambdaGenerator(
    const char *const base_nodeLabels[],
    vector<NodeConditionT> query_nodeConditions) {
  return [base_nodeLabels, query_nodeConditions](VertexId query_nodeId,
                                                 VertexId base_nodeId) -> bool {
    auto res = false;
    auto condForQueryNode =
        getQueryNodeConditionById(query_nodeConditions, query_nodeId);
    auto metadataForBaseNode =
        getBaseNodeMetadataById(base_nodeLabels, base_nodeId);

    switch (condForQueryNode.cond) {
      case StrConditionT_WILDCARD:
        res = true;
        break;
      case StrConditionT_EQ:
        return condForQueryNode.operand == metadataForBaseNode;
        break;
      case StrConditionT_NEQ:
        return condForQueryNode.operand != metadataForBaseNode;
        break;
      default:
        assert(false);
    };
    return res;
  };
}

// Integer comparison operators
std::function<bool(VertexId, VertexId, VertexId, VertexId)> edgeLambdaGenerator(
    vector<EdgeLabelT> base_edgeLabels,
    vector<EdgeConditionT> query_edgeConditions) {
  return
      [base_edgeLabels, query_edgeConditions](
          VertexId qsrc, VertexId qdest, VertexId src, VertexId dest) -> bool {
        auto res = false;
        auto condForQueryEdge =
            getQueryEdgeConditionById(query_edgeConditions, qsrc, qdest);
        auto metadataForBaseEdge =
            getBaseEdgeMetadataById(base_edgeLabels, src, dest);

        switch (condForQueryEdge.cond) {
          case IntConditionT_GT:
            return metadataForBaseEdge.label > condForQueryEdge.operand;
            break;
          default:
            assert(false);
        };
        return res;
      };
}

int main2() {
  // Suppose our graph has 2 nodes (id 0, 1) with string labels
  const char *base_nodeLabels[] = {"davis", "berkeley"};

  // In C-land, we pass query conditions (for nodes) as an array of `NodeConditionT` structs
  NodeConditionT query_node_conditions_1[] = {node_eql_to_davis(0), // label for id=0 match davis
                                              node_neq_to_berkeley(1)}; // label for id=1 not match berkeley

  // For convenience, I convert the array of structs to a vector
  vector<NodeConditionT> query_node_conditions_vector(
      query_node_conditions_1, query_node_conditions_1 + 2);

  // Obtain a NodeMatcher lambda
  auto node_lambda_1 =
      nodeLambdaGenerator(base_nodeLabels, query_node_conditions_vector);

  // Tests for the generated match functions:
  assert(node_lambda_1(0, 0));
  assert(!node_lambda_1(1, 1));

  assert(!node_lambda_1(0, 1));
  assert(node_lambda_1(1, 0));

  EdgeLabelT base_edgeLabels[] = {edgeLabel(0, 1, 30), edgeLabel(3, 4, 45)};
  EdgeConditionT query_edgeConditions[] = {edge_gt_n(0, 1, 42)};

  auto edge_lambda_1 = edgeLambdaGenerator(
      vector<EdgeLabelT>(base_edgeLabels, base_edgeLabels + 2),
      vector<EdgeConditionT>(query_edgeConditions, query_edgeConditions + 1));

  // Check to see if the condition between 0--1 in query graph holds for 3--4 in
  // base graph

  // This assertion should hold because the label is 45
  assert(edge_lambda_1(0, 1, 3, 4));
  // The label between 0--1 is 30
  assert(!edge_lambda_1(0, 1, 0, 1));
}

/*
 * Compute B += A for CSR matrix A, C-contiguous dense matrix B
 *
 * Input Arguments:
 *   I  n_row           - number of rows in A
 *   I  n_col           - number of columns in A
 *   I  Ap[n_row+1]     - row pointer
 *   I  Aj[nnz(A)]      - column indices
 *   T  Ax[nnz(A)]      - nonzero values
 *   T  Bx[n_row*n_col] - dense matrix in row-major order
 *
 */
template <class I, class T>
void csr_todense(const I n_row,
                 const I n_col,
                 const I Ap[],
                 const I Aj[],
                 // const T Ax[],
                       T Bx[])
{
    T * Bx_row = Bx;
    for(I i = 0; i < n_row; i++){
        for(I jj = Ap[i]; jj < Ap[i+1]; jj++){
            // Bx_row[Aj[jj]] += Ax[jj];
            Bx_row[Aj[jj]] = 1;
        }
        Bx_row += n_col;
    }
}


// Default print_callback
template <typename Graph1,
          typename Graph2>
struct vector_callback {

  vector<int>& vec;

  vector_callback(const Graph1& graph1, const Graph2& graph2, vector<int>& vec)
    : graph1_(graph1), graph2_(graph2), vec(vec) {}

  template <typename CorrespondenceMap1To2,
            typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) {

    // Print (sub)graph isomorphism map
    /*
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1)
      std::cout << '(' << get(vertex_index_t(), graph1_, v) << ", "
                << get(vertex_index_t(), graph1_, get(f, v)) << ") ";
    */

    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) {
      vec.push_back(get(vertex_index_t(), graph1_, v));
      vec.push_back(get(vertex_index_t(), graph1_, get(f, v)));
    }

    // std::cout << std::endl;

    return true;
  }
private:
  const Graph1& graph1_;
  const Graph2& graph2_;
};


extern "C" {

double sm_cpp(
 const int num_nodes,
 const int num_edges,
 const int *row_offsets,
 const int *col_indices,
 const int num_query_nodes,
 const int num_query_edges,
 const int *query_row_offsets,
 const int *query_col_indices,
 const int num_runs,
 //
 const char *base_nodeLabels[],
 const NodeConditionT *query_node_conditions,
 //
 EdgeLabelT *base_edgeLabels,
 //
 EdgeConditionT *query_edgeConditions,
 //
 int *subgraphs_count,
 int **subgraphs_mappings
 ) {

  /*
  cout << sizeof(VertexId) << sizeof(StrConditionT) << sizeof(StrConditionT_EQ)
       << sizeof("h") << endl;
  cout << "num_nodes:" << num_nodes << endl;
  cout << "num_edges:" << num_edges << endl;
  cout << "row_offsets:" << row_offsets << endl;
  cout << "col_indices:" << col_indices << endl;
  cout << "num_query_nodes:" << num_query_nodes << endl;
  cout << "num_query_edges:" << num_query_edges << endl;
  cout << "query_row_offsets:" << query_row_offsets << endl;
  cout << "query_col_indices:" << query_col_indices << endl;
  cout << "num_runs:" << num_runs << endl;
  cout << "subgraphs:" << subgraphs << endl;
  cout << "------" << endl;

  cout << "base_nodeLabels: " << base_nodeLabels[0] << " " << base_nodeLabels[1]
       << endl;

  for (int i = 0; i < num_query_nodes; i++) {
    cout << "query_node_condition " << i << ": "
         << toString(query_node_conditions[i]) << endl;
  }
  cout << "base_edgeLabels: " << toString(base_edgeLabels[0]) << endl;
  cout << "query_edgeConditions: " << toString(query_edgeConditions[0]) << endl;
  */


  typedef adjacency_list<setS, vecS, bidirectionalS> graph_type;

  // Build graph1
  graph_type boost_query_graph(num_query_nodes);
  // add_edge(0, 6, graph1); add_edge(0, 7, graph1);
  // add_edge(1, 5, graph1); add_edge(1, 7, graph1);
  // add_edge(2, 4, graph1); add_edge(2, 5, graph1); add_edge(2, 6, graph1);
  // add_edge(3, 4, graph1);

  // // Build graph2
  // int num_vertices2 = 9;
  // graph_type graph2(num_vertices2);
  // add_edge(0, 6, graph2); add_edge(0, 8, graph2);
  // add_edge(1, 5, graph2); add_edge(1, 7, graph2);
  // add_edge(2, 4, graph2); add_edge(2, 7, graph2); add_edge(2, 8, graph2);
  // add_edge(3, 4, graph2); add_edge(3, 5, graph2); add_edge(3, 6, graph2);

  int dense[num_query_nodes*num_query_nodes];
  memset(dense, 0, num_query_nodes*num_query_nodes*sizeof(int));
  csr_todense(num_query_nodes, num_query_nodes, query_row_offsets, query_col_indices, dense);

  for(int i=0; i < num_query_nodes; i++) {
    for(int j=0; j < num_query_nodes; j++) {
      cout << dense[i*num_query_nodes+j] << " ";
      if(dense[i*num_query_nodes+j] != 0) {
        add_edge(i, j, boost_query_graph);
      }
    }
    cout << endl;
  }

  graph_type boost_base_graph = boost_query_graph;

  // Create callback to print mappings
  // vf2_print_callback<graph_type, graph_type> callback(boost_query_graph, boost_base_graph);

  auto vec = vector<int>();

  vector_callback<graph_type, graph_type> callback(boost_query_graph, boost_base_graph, vec);

  // Print out all subgraph isomorphism mappings between graph1 and graph2.
  // Vertices and edges are assumed to be always equivalent.
  vf2_subgraph_mono(boost_query_graph, boost_base_graph, callback);

  *subgraphs_count = callback.vec.size();

  int *result = new int[callback.vec.size()];
  for(int i=0; i<callback.vec.size(); i++) {
    result[i] = callback.vec[i];
  }

  *subgraphs_mappings = result;

  return 42;
}
}
