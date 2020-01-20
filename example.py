from ctypes import *
import struct

lib = cdll.LoadLibrary('./libgunrock.so')

row_list = [0, 3, 6, 11, 15, 19, 23, 26]
col_list = [1, 2, 3, 0, 2, 4, 0, 1, 3, 4, 5, 0, 2, 5, 6, 1, 2, 5, 6, 2, 3, 4, 6, 3, 4, 5]

nodes = len(row_list) - 1
edges = len(col_list)

node = pointer((c_int * nodes)())


qrow_list = row_list[:]
qcol_list = col_list[:]

qnodes = len(qrow_list) - 1
qedges = len(qcol_list)

row = pointer((c_int * len(row_list))(*row_list))
col = pointer((c_int * len(col_list))(*col_list))

qrow = pointer((c_int * len(qrow_list))(*qrow_list))
qcol = pointer((c_int * len(qcol_list))(*qcol_list))

# NodeConditionT node_eql_to_davis(VertexId nodeId) {
#   NodeConditionT res;
#   res.node = nodeId;
#   res.cond = StrConditionT_EQ;
#   res.operand = "davis";
#   return res;
# }

def NodeConditionT(node, cond, operand):
    # struct NodeConditionT {
    #   VertexId node;  // VertexId in the query graph
    #   StrConditionT cond;
    #   const char *operand;
    # };

    return struct.pack("i i 32s", node, cond, operand)
    # return struct.pack("ii", node, cond)
    # return struct.pack("iic", node, cond, "abc")
    # return struct.pack("i", node)

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
    return struct.pack("iiii", src, dest, 3, n)

base_nodeLabels = ["davis", "berkeley"]
c_base_nodeLabels = (c_char_p * len(base_nodeLabels))()
c_base_nodeLabels[:] = base_nodeLabels

query_node_conditions = [node_eql_to_davis(42), node_neq_to_berkeley(45)]
c_query_node_conditions = "".join(query_node_conditions)

base_edgeLabels = [edgeLabel(0, 1, 30), edgeLabel(3, 4, 45)]
c_base_edgeLabels = "".join(base_edgeLabels)

query_edgeConditions = [edge_gt_n(0, 1, 42)]
c_query_edgeConditions = "".join(query_edgeConditions)

elapsed = lib.sm_cpp(nodes, edges, row, col, qnodes, qedges, qrow, qcol, 1, c_base_nodeLabels, 
                    c_query_node_conditions, c_base_edgeLabels, c_query_edgeConditions, node)
