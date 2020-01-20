#include "lambda.hh"
#include <sstream>

using namespace std;

string toString(EdgeConditionT cond) {
  stringstream ss;
  ss << "#EdgeCondition<" << cond.src << ", " << cond.dest << ", " << cond.cond << ", " << cond.operand << ">";
  return ss.str();
}

string toString(NodeConditionT cond) {
  stringstream ss;
  ss << "#NodeCondition<" << cond.node << ", " << cond.cond << ", " << cond.operand << ">";
  return ss.str();
}

string toString(EdgeLabelT edge) {
  stringstream ss;
  ss << "#EdgeLabel<" << edge.src << ", " << edge.dest << ", " << edge.label << ">";
  return ss.str();
}