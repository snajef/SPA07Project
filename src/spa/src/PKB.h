#pragma once
#include "CFG.h"
#include "CFGBip.h"
#include "Table.h"
#include "Util.h"
#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

class PKB {
private:
  // Range of statements from first (inclusive) and last (exclusive)
  typedef std::pair<int, int> StmtRange;

  // General entities
  int stmtCount = 0;
  Table varTable{1};
  std::map<std::string, StmtRange> procTable;
  Table constTable{1};
  std::map<StatementType, std::set<int>> stmtTable;
  CFG cfg;
  CFGBip cfgBip;
  std::map<int, std::pair<std::string, std::vector<std::string>>> assignMap;
  std::map<int, StatementType> stmtMap;
  Table procExitStmtTable{2};

  // PQL Relation Tables
  Table followsTable{2};
  Table followsTTable{2};
  Table parentTable{2};
  Table parentTTable{2};
  Table usesPTable{2};
  Table usesSTable{2};
  Table modifiesPTable{2};
  Table modifiesSTable{2};
  Table callsTable{2};
  Table callsTTable{2};
  Table nextTable{2};
  Table nextBipTable{2};

  // Other Relation Tables
  Table callProcNameTable{2};

  // Pattern Tables
  std::map<int, std::pair<std::string, std::string>> assignTable;
  Table whileTable{2};
  Table ifTable{2};

public:
  // Setters
  // Entity setters
  void setVar(const std::string &);
  void setProc(const std::string &, int, int);
  void setStmtType(int, StatementType);
  void setConst(int);
  void setProcExitStmt(std::string, std::vector<int>);
  // Relation setters
  void setFollows(int, int);
  void setFollowsT(int, int);
  void setParent(int, int);
  void setParentT(int, int);
  void setUses(int, std::string);
  void setUses(std::string, std::string);
  void setModifies(int, std::string);
  void setModifies(std::string, std::string);
  void setCalls(std::string, std::string);
  void setCallsT(std::string, std::string);
  void setNext(int, int);
  void setNextBip(Table);
  void setNextBipT(Table);
  void setAffectsBip(Table);
  // Pattern setters
  void setAssign(int, std::string &, std::string &);
  void setIf(int, const std::string &);
  void setWhile(int, const std::string &);
  // Other setters
  void setCallProcName(int, const std::string &);
  void setCFG(CFG &);
  void setCFGBip(CFGBip &);
  void setAssignMap(int, std::string);
  void setStmtMap(int, StatementType);

  // Getters
  // Entity getter
  Table getVarTable() const;
  Table getProcTable() const;
  Table getConstTable() const;
  Table getStmtType(StatementType);
  Table getProcStmt();
  Table getProcExitStmt() const;
  // Relation getter
  Table getFollows() const;
  Table getFollowsT() const;
  Table getParent() const;
  Table getParentT() const;
  Table getUsesP() const;
  Table getUsesS() const;
  Table getModifiesP() const;
  Table getModifiesS() const;
  Table getCalls() const;
  Table getCallsT() const;
  Table getNext() const;
  Table getNextBip() const;

  bool isNextT(int, int);
  Table getNextT(int, bool);
  Table getNextT();
  bool isAffects(int, int);
  Table getAffects(int, bool);
  Table getAffects();
  Table getAffectsT();
  Table getNextBipT();
  Table getAffectsBip();
  Table getAffectsBipT();

  Table getCallProcNameTable() const;
  // Pattern getter
  Table getAssignMatches(std::string expr, bool partial);
  Table getWhileMatches();
  Table getIfMatches();
  // Other getters
  Table getCallProcName();
  CFG getCFG();
  CFGBip getCFGBip();
  int getStmtCount();

  void clearCache();
};
