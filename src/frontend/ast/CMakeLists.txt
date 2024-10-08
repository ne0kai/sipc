add_library(ast)
target_sources(
  ast
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ASTVisitor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/SyntaxTree.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/SyntaxTree.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/AST.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAccessExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAccessExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAllocExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAllocExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayAccessExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayAccessExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayLengthExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayLengthExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayOfExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTArrayOfExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAssignStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTAssignStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBinaryExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBinaryExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBlockStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBlockStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBooleanExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTBooleanExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeRefExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeRefExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeclNode.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeclNode.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeclStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTDeclStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTErrorStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTErrorStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFieldExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFieldExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTForIteratorStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTForIteratorStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTForRangeStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTForRangeStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFunAppExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFunAppExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFunction.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTFunction.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTIfStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTIfStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTInputExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTInputExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNegExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNegExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNode.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNotExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNotExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNullExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNullExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNumberExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTNumberExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTOutputStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTOutputStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTPostfixStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTPostfixStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTProgram.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTProgram.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTRecordExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTRecordExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTRefExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTRefExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTReturnStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTReturnStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTTernaryExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTTernaryExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTVariableExpr.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTVariableExpr.h
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTWhileStmt.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes/ASTWhileStmt.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ASTVisitor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/ASTBuilder.h
          ${CMAKE_SOURCE_DIR}/externals/PicoSHA2/picosha2.h)
target_include_directories(
  ast
  PRIVATE ${ANTLR_TIPGrammar_OUTPUT_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/treetypes
          ${CMAKE_SOURCE_DIR}/externals/PicoSHA2
          ${CMAKE_SOURCE_DIR}/src/frontend/prettyprint
          ${CMAKE_SOURCE_DIR}/src/frontend/iterators)
target_link_libraries(ast PRIVATE antlr4_static antlrgen codegen iterators
                                  coverage_config)
