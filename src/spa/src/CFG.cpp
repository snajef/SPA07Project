#include "CFG.h"
#include "assert.h"
#include <queue>
#include <stack>

CFG::CFG() {}

CFG::CFG(Table procStmtTable, Table nextTable, Table whileIfTable,
         int stmtCount) {
  // Populate initialGraph from nextTable (1-based indexing)
  initialGraph.resize(stmtCount + 1);
  for (auto data : nextTable.getData()) {
    int u = std::stoi(data[0]);
    int v = std::stoi(data[1]);
    initialGraph[u].push_back(v);
  }
  // Populate inDegree
  std::vector<int> inDegree(stmtCount + 1, 0);
  for (int i = 1; i < initialGraph.size(); ++i) {
    for (auto j : initialGraph[i]) {
      inDegree[j]++;
    }
  }
  for (auto data : procStmtTable.getData()) {
    int start = std::stoi(data[1]);
    numCompressedNodes++;
    // Procedure only has one statement
    if (initialGraph[start].empty()) {
      initialToCompressed[start] = numCompressedNodes;
    } else {
      populateInitialToCompressed(start, whileIfTable, inDegree);
    }
  }
  // Populate forward and reverse CompressedGraph
  populateCompressedToInitial();
  forwardCompressedGraph.resize(numCompressedNodes + 1);
  reverseCompressedGraph.resize(numCompressedNodes + 1);
  populateCompressedGraph(procStmtTable);
}

void CFG::populateInitialToCompressed(int start, Table whileIfTable,
                                      std::vector<int> inDegree) {
  // Iterative DFS to populate initialToCompressed
  std::vector<bool> visited(initialGraph.size() + 1, false);
  std::stack<int> stack;
  stack.push(start);
  initialToCompressed[start] = numCompressedNodes;

  while (!stack.empty()) {
    int u = stack.top();
    stack.pop();
    if (!visited[u]) {
      visited[u] = true;
    }

    for (auto v : initialGraph[u]) {
      if (!visited[v]) {
        stack.push(v);

        if (whileIfTable.contains({std::to_string(v)})) {
          // Child is while or if statement
          numCompressedNodes++;
          initialToCompressed[v] = numCompressedNodes;
        } else if (inDegree[v] > 1) {
          // Child has in degree > 1
          numCompressedNodes++;
          initialToCompressed[v] = numCompressedNodes;
        } else {
          // Parent is while or if statement
          if (whileIfTable.contains({std::to_string(u)})) {
            numCompressedNodes++;
            initialToCompressed[v] = numCompressedNodes;
          } else {
            initialToCompressed[v] = initialToCompressed[u];
          }
        }
      }
    }
  }
}

void CFG::populateCompressedToInitial() {
  for (auto i : initialToCompressed) {
    compressedToInitial[i.second].push_back(i.first);
  }
}

void CFG::populateCompressedGraph(Table procStmtTable) {
  // Traverse the initialGraph and populate forward and reverse CompressedGraph
  // using initialToCompressed
  std::vector<bool> visited(initialGraph.size() + 1, false);
  std::queue<int> queue;
  // Mark start of each procedure as visited and push into queue for traversal
  for (auto data : procStmtTable.getData()) {
    int start = std::stoi(data[1]);
    queue.push(start);
    visited[start] = true;
  }
  while (!queue.empty()) {
    int u = queue.front();
    queue.pop();
    for (auto v : initialGraph[u]) {
      if (!visited[v]) {
        queue.push(v);
        visited[v] = true;
      }
      if (initialToCompressed[v] != initialToCompressed[u]) {
        forwardCompressedGraph[initialToCompressed[u]].push_back(
            initialToCompressed[v]);
        reverseCompressedGraph[initialToCompressed[v]].push_back(
            initialToCompressed[u]);
      }
    }
  }
}

std::vector<int> CFG::getNextTForward(int start, int end = -1) const {
  std::vector<int> result;
  std::vector<std::vector<int>> compressedCFG = forwardCompressedGraph;
  bool reachedEnd = false;

  // initialize visited array
  std::vector<bool> visited(initialToCompressed.size() + 1, false);
  visited[0] = true; // no stmt line 0

  // adding all elements in start node
  int startNode = initialToCompressed.at(start);
  std::vector<int> linesInNode = compressedToInitial.at(startNode);
  int index = start - linesInNode[0] + 1;
  for (int i = index; i < linesInNode.size(); i++) {
    result.push_back(linesInNode[i]);
    visited[linesInNode[i]] = true;
    if (linesInNode[i] == end) {
      reachedEnd = true;
    }
  }

  std::queue<int> q;
  q.push(startNode);
  while (!q.empty()) {
    int curr = q.front();
    q.pop();

    for (int i : compressedCFG[curr]) {
      if (!visited[compressedToInitial.at(i)[0]]) {
        for (int j : compressedToInitial.at(
                 i)) { // adding all elements within current node
          if (!visited[j]) {
            visited[j] = true;
            result.push_back(j);
            if (j == end) { // for nextT with 2 constants
              reachedEnd = true;
              break;
            }
          }
        }
        q.push(i);
      }
    }
  }

  // for nextT with two constants that did not reach end line
  if ((end > 0) && (!reachedEnd)) {
    // return empty vector if end not reachable from start
    return std::vector<int>{};
  }

  return result;
}

std::vector<int> CFG::getNextTReverse(int start) const {
  std::vector<int> result;
  std::vector<std::vector<int>> compressedCFG = reverseCompressedGraph;

  std::vector<bool> visited(initialToCompressed.size() + 1, false);
  visited[0] = true;

  // adding all elements in start node
  int startNode = initialToCompressed.at(start);
  std::vector<int> linesInNode = compressedToInitial.at(startNode);
  int index = start - linesInNode[0] - 1;

  for (int i = index; i >= 0; i--) {
    result.push_back(linesInNode[i]);
    visited[linesInNode[i]] = true;
  }

  std::queue<int> q;
  q.push(startNode);
  while (!q.empty()) {
    int curr = q.front();
    q.pop();
    for (int i : compressedCFG[curr]) {
      if (!visited[compressedToInitial.at(
              i)[compressedToInitial.at(i).size() - 1]]) {
        for (int j : compressedToInitial.at(
                 i)) { // adding all elements within current node
          if (!visited[j]) {
            visited[j] = true;
            result.push_back(j);
          }
        }
        q.push(i);
      }
    }
  }

  return result;
}

Table CFG::getNextT() const {
  Table table{2};

  for (int i = 1; i < initialToCompressed.size() + 1; i++) {
    std::vector<int> result = getNextTForward(i);
    for (int j : result) {
      table.insertRow({std::to_string(i), std::to_string(j)});
    }
  }
  return table;
}

Table CFG::getNextT(int start, bool isForward) const {
  Table table{1};
  std::vector<int> result;
  if (isForward) {
    result = getNextTForward(start);
  } else {
    result = getNextTReverse(start);
  }

  for (int i : result) {
    table.insertRow({std::to_string(i)});
  }
  return table;
}

bool CFG::isNextT(int start, int end) const {
  return getNextTForward(start, end).size() > 0;
}

std::vector<int> CFG::getAffectsForward(int start, std::string v,
                                        Table modifiesTable,
                                        Table usesAssignTable) const {
  std::vector<std::vector<int>> compressedCFG = forwardCompressedGraph;
  std::vector<int> results;

  // Initialize visited array
  std::vector<bool> visited(initialToCompressed.size() + 1, false);
  visited[0] = true; // no stmt line 0

  // Add lines in start node to results if modified some variable
  int startNode = initialToCompressed.at(start);
  std::vector<int> linesInNode = compressedToInitial.at(startNode);
  int index = start - linesInNode[0] + 1;
  for (int i = index; i < linesInNode.size(); i++) {
    int node = linesInNode[i];
    if (usesAssignTable.contains({std::to_string(node), v})) {
      results.push_back(node);
    }
    if (modifiesTable.contains({std::to_string(node), v}) &&
        i != linesInNode.size() - 1) {
      // v is modified in a line in the start node
      // that is not the last line of start node
      return results;
    }
    visited[node] = true;
  }

  std::queue<int> q;
  q.push(startNode);
  while (!q.empty()) {
    int curr = q.front();
    q.pop();

    // Looping through neighbor nodes
    for (int i : compressedCFG[curr]) {
      if (!visited[compressedToInitial.at(i)[0]]) {
        bool isModified = false;
        for (int j : compressedToInitial.at(i)) {
          if (isModified) {
            visited[j] = true;
          }
          if (!visited[j]) {
            visited[j] = true;
            // Add to results if Uses(j, v) is true
            if (usesAssignTable.contains({std::to_string(j), v})) {
              results.push_back(j);
            }
            if (modifiesTable.contains({std::to_string(j), v})) {
              isModified = true;
            }
          }
        }
        // Enqueue only if no line in compressed node modifies v
        if (!isModified) {
          q.push(i);
        }
      }
    }
  }
  return results;
}

std::vector<int> CFG::getAffectsReverse(int start, std::string v,
                                        Table modifiesTable,
                                        Table modifiesAssignTable) const {
  std::vector<std::vector<int>> compressedCFG = reverseCompressedGraph;
  std::vector<int> results;

  // Initialize visited array
  std::vector<bool> visited(initialToCompressed.size() + 1, false);
  visited[0] = true; // no stmt line 0

  // Add lines in start node to results if modified some variable
  int startNode = initialToCompressed.at(start);
  std::vector<int> linesInNode = compressedToInitial.at(startNode);
  int index = start - linesInNode[0] - 1;
  for (int i = index; i >= 0; i--) {
    int node = linesInNode[i];
    if (modifiesAssignTable.contains({std::to_string(node), v})) {
      results.push_back(node);
      return results;
    }
    visited[node] = true;
  }

  std::queue<int> q;
  q.push(startNode);
  while (!q.empty()) {
    int curr = q.front();
    q.pop();

    // Looping through neighbor nodes
    for (int i : compressedCFG[curr]) {
      if (!visited[compressedToInitial.at(i)[0]]) {
        bool isModified = false;
        auto lines = compressedToInitial.at(i);
        for (int j = lines.size() - 1; j >= 0; j--) {
          int line = lines[j];
          if (isModified) {
            visited[line] = true;
          }
          if (!visited[line]) {
            visited[line] = true;
            // Add to results if Modifies(line, v) and line is assignment stmt
            if (modifiesAssignTable.contains({std::to_string(line), v})) {
              results.push_back(line);
            }
            if (modifiesTable.contains({std::to_string(line), v})) {
              isModified = true;
              break;
            }
          }
        }
        // Enqueue only if no line in compressed node modifies v
        if (!isModified) {
          q.push(i);
        }
      }
    }
  }
  return results;
}

bool CFG::isAffects(int a1, int a2, Table usesTable,
                    Table modifiesTable) const {
  // Get variable modified in line a1 and used in line a2
  modifiesTable.setHeader({"a1", "v"});
  usesTable.setHeader({"a2", "v"});
  auto modifiesA1Table = modifiesTable.filter("a1", {std::to_string(a1)});
  auto usesA2Table = usesTable.filter("a2", {std::to_string(a2)});
  modifiesA1Table.mergeWith(usesA2Table);

  if (modifiesA1Table.empty()) {
    // No variable modified in line a1 and used in line a2
    return false;
  } else {
    // Continue to get variable
    assert(modifiesA1Table.size() == 1);
    std::string v;
    for (auto data : modifiesA1Table.getData({"v"})) {
      v = data[0];
    }
    // Checking if there exist any s that modifies v in the interval (a1, a2)
    for (int i = a1 + 1; i < a2; i++) {
      if (modifiesTable.contains({std::to_string(i), v})) {
        return false;
      }
    }
    return true;
  }
}

Table CFG::getAffects(int start, bool isForward, Table usesTable,
                      Table modifiesTable, std::set<int> assignStmts) const {
  Table table{1};
  std::vector<int> result;
  std::vector<std::string> assignStmtsVec;
  for (auto i : assignStmts) {
    assignStmtsVec.push_back(std::to_string(i));
  }

  if (isForward) {
    // Affects(k, a) utilizes forwardCompressedGraph, k here is a constant
    modifiesTable.setHeader({"a1", "v"});
    auto modifiesATable = modifiesTable.filter("a1", {std::to_string(start)});
    // modifiesATable has one variable after filter
    assert(modifiesATable.size() == 1);
    std::string v;
    for (auto data : modifiesATable.getData({"v"})) {
      v = data[0];
    }
    usesTable.setHeader({"a2", "v"});
    auto usesAssignTable = usesTable.filter("a2", assignStmtsVec);
    result = getAffectsForward(start, v, modifiesTable, usesAssignTable);
  } else {
    // Affects(a, k) utilizes reverseCompressedGraph, k here is a constant
    usesTable.setHeader({"a2", "v"});
    auto usesATable = usesTable.filter("a2", {std::to_string(start)});
    // usesATable has one or more variables after filter
    std::vector<std::string> variables;
    for (auto data : usesATable.getData({"v"})) {
      variables.push_back(data[0]);
    }
    modifiesTable.setHeader({"a1", "v"});
    auto modifiesAssignTable = modifiesTable.filter("a1", assignStmtsVec);
    for (auto v : variables) {
      auto temp =
          getAffectsReverse(start, v, modifiesTable, modifiesAssignTable);
      result.insert(result.end(), temp.begin(), temp.end());
    }
  }
  for (int i : result) {
    table.insertRow({std::to_string(i)});
  }
  return table;
}

Table CFG::getAffects(Table usesTable, Table modifiesTable,
                      std::set<int> assignStmts) const {
  Table table{2};
  std::vector<std::string> assignStmtsVec;
  for (auto i : assignStmts) {
    assignStmtsVec.push_back(std::to_string(i));
  }
  modifiesTable.setHeader({"a1", "v"});
  usesTable.setHeader({"a2", "v"});
  auto modifiesAssignTable = modifiesTable.filter("a1", assignStmtsVec);
  auto usesAssignTable = usesTable.filter("a2", assignStmtsVec);
  for (auto data : modifiesAssignTable.getData()) {
    auto result = getAffectsForward(std::stoi(data[0]), data[1], modifiesTable,
                                    usesAssignTable);
    for (int i : result) {
      table.insertRow({data[0], std::to_string(i)});
    }
  }
  return table;
}
