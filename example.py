from ctypes import *
import ctypes
import struct

lib = cdll.LoadLibrary('./libgunrock.so')

# Example matrix in CSR format

row_list = [0, 3, 6, 11, 15, 19, 23, 26]
col_list = [1, 2, 3, 0, 2, 4, 0, 1, 3, 4, 5, 0, 2, 5, 6, 1, 2, 5, 6, 2, 3, 4, 6, 3, 4, 5]

# We need to calculate number of nodes/edges of the base graph so we can pass to C API

nodes = len(row_list) - 1
edges = len(col_list)

node = pointer((c_int * nodes)())

# Likewise, calculate number of nodes/edges of the query graph, then pass to C

qrow_list = row_list[:]
qcol_list = col_list[:]

qnodes = len(qrow_list) - 1
qedges = len(qcol_list)

row = pointer((c_int * len(row_list))(*row_list))
col = pointer((c_int * len(col_list))(*col_list))

qrow = pointer((c_int * len(qrow_list))(*qrow_list))
qcol = pointer((c_int * len(qcol_list))(*qcol_list))

def NodeConditionT(node, cond, operand):
    # This is how NodeConditionT is represented as a struct in C

    # struct NodeConditionT {
    #   VertexId node;  // VertexId in the query graph
    #   StrConditionT cond;
    #   const char *operand; //In fact, it must be `char operand[32];` otherwise you get SEGFAULT
    # };

    # First element of the struct is a 32-bit integer
    # Second element of the struct is an enum
    # Third element is an array of char with maximum 32 elements.
    return struct.pack("i i 32s", node, cond, operand)

def node_eql_to_davis(nodeId):
    # return struct.pack(nodeId, 1, "davis")
    return NodeConditionT(nodeId, 1, "davis")

def node_neq_to_berkeley(nodeId):
    # return struct.pack(nodeId, 2, "berkeley")
    return NodeConditionT(nodeId, 2, "berkeley")

# struct EdgeLabelT {
#   VertexId src;
#   VertexId dest;
#   int label;
# };

def edgeLabel(src, dest, label):
    return struct.pack("iii", src, dest, label)

# struct EdgeConditionT {
#   VertexId src;
#   VertexId dest;
#   IntConditionT cond;
#   int operand;
# };

def edge_gt_n(src, dest, n):
    return struct.pack("iiii", src, dest, 3, n) # edge between src and dst > n. enum for ">" is 3.

# Attach node labels in the base graph
# The order of the array corresponds to `node_id`s in Gunrock, ordered from 0..(n-1)
base_nodeLabels = ["davis", "berkeley"]
c_base_nodeLabels = (c_char_p * len(base_nodeLabels))()
c_base_nodeLabels[:] = base_nodeLabels

# Declare conditions on the edges of the base graph

# for the query graph, set the following conditions
#     * on node with id 42,
#     * on node with id 45
query_node_conditions = [node_eql_to_davis(42), node_neq_to_berkeley(45)]

# pack an array of structs into one binary chunk
c_query_node_conditions = "".join(query_node_conditions)

# Labels on edges in the base graph:

# Base graph may have integer as node label
base_edgeLabels = [edgeLabel(0, 1, 30), edgeLabel(3, 4, 45)]
# pack an array of structs into one binary chunk
c_base_edgeLabels = "".join(base_edgeLabels)

query_edgeConditions = [edge_gt_n(0, 1, 42)]
# pack an array of structs into one binary chunk
c_query_edgeConditions = "".join(query_edgeConditions)

# Call sm_cpp defined in
# https://github.com/johari/gunrock-c-python-sm/blob/eaf3cebe50f41f4345182958cca63705793e8d07/lambda.cpp#L180-L220
def my_callback(one, two):
    print one
    print two

subgraphs_count    = ctypes.c_int()
subgraphs_mappings = ctypes.POINTER(ctypes.c_int)()

# elapsed = lib.sm_cpp(nodes, edges, row, col, qnodes, qedges, qrow, qcol, 1, c_base_nodeLabels,
#                     c_query_node_conditions, c_base_edgeLabels, c_query_edgeConditions, node)

elapsed = lib.sm_cpp(
    nodes, edges, row, col, qnodes, qedges, qrow, qcol, 1,
    c_base_nodeLabels,
    c_query_node_conditions,
    c_base_edgeLabels,
    c_query_edgeConditions,
    byref(subgraphs_count),
    byref(subgraphs_mappings)
)

for i in range(0, subgraphs_count.value, 2):
    print(subgraphs_mappings[i], subgraphs_mappings[i+1])

print "elapsed %f" % elapsed