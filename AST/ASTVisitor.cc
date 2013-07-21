#include <cstdio>
#include <cstdlib>
#include <clang-c/Index.h>

void show_spell(const CXCursor &cursor) {
  CXString spell = clang_getCursorSpelling(cursor);
  printf("  Text: %s\n", clang_getCString(spell));
  clang_disposeString(spell);
}

void show_type(const CXCursor &cursor) {
  CXType type = clang_getCursorType(cursor);
  CXString typeName = clang_getTypeSpelling(type);
  CXTypeKind typeKind = type.kind;
  CXString typeKindName = clang_getTypeKindSpelling(typeKind);
  printf("  Type: %s\n", clang_getCString(typeName));
  printf("  TypeKind: %s\n", clang_getCString(typeKindName));
  clang_disposeString(typeName);
  clang_disposeString(typeKindName);
}

void show_linkage(const CXCursor &cursor) {
  CXLinkageKind linkage = clang_getCursorLinkage(cursor);
  const char *linkageName;
  switch (linkage) {
    case CXLinkage_Invalid:        linkageName = "Invalid"; break;
    case CXLinkage_NoLinkage:      linkageName = "NoLinkage"; break;
    case CXLinkage_Internal:       linkageName = "Internal"; break;
    case CXLinkage_UniqueExternal: linkageName = "UniqueExternal"; break;
    case CXLinkage_External:       linkageName = "External"; break;
    default:                       linkageName = "Unknown"; break;
  }
  printf("  Linkage: %s\n", linkageName);
}

void show_parent(const CXCursor &cursor, const CXCursor &parent) {
  CXCursor semaParent = clang_getCursorSemanticParent(cursor);
  CXCursor lexParent  = clang_getCursorLexicalParent(cursor);
  CXString parentName = clang_getCursorSpelling(parent);
  CXString semaParentName = clang_getCursorSpelling(semaParent);
  CXString lexParentName = clang_getCursorSpelling(lexParent);
  printf("  Parent: parent:%s semantic:%s lexicial:%s\n",
         clang_getCString(parentName),
         clang_getCString(semaParentName),
         clang_getCString(lexParentName));

  clang_disposeString(parentName);
  clang_disposeString(semaParentName);
  clang_disposeString(lexParentName);
}

void show_location(const CXCursor &cursor) {
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  CXFile file;
  unsigned line, column, offset;
  clang_getSpellingLocation(loc, &file, &line, &column, &offset);
  CXString fileName = clang_getFileName(file);
  printf("  Location: %s:%d:%d:%d\n",
         clang_getCString(fileName), line, column, offset);
  clang_disposeString(fileName);
}

void show_usr(const CXCursor &cursor) {
  CXString usr = clang_getCursorUSR(cursor);
  printf("  USR: %s\n", clang_getCString(usr));
  clang_disposeString(usr);
}

void show_cursor_kind(const CXCursor &cursor) {
  CXCursorKind curKind  = clang_getCursorKind(cursor);
  CXString curKindName  = clang_getCursorKindSpelling(curKind);

  const char *type;
  if (clang_isAttribute(curKind))            type = "Attribute";
  else if (clang_isDeclaration(curKind))     type = "Declaration";
  else if (clang_isExpression(curKind))      type = "Expression";
  else if (clang_isInvalid(curKind))         type = "Invalid";
  else if (clang_isPreprocessing(curKind))   type = "Preprocessing";
  else if (clang_isReference(curKind))       type = "Reference";
  else if (clang_isStatement(curKind))       type = "Statement";
  else if (clang_isTranslationUnit(curKind)) type = "TranslationUnit";
  else if (clang_isUnexposed(curKind))       type = "Unexposed";
  else                                       type = "Unknown";

  printf("  CursorKind: %s\n", clang_getCString(curKindName));
  printf("  CursorKindType: %s\n", type);
  clang_disposeString(curKindName);
}

void show_included_file(const CXCursor &cursor) {
  CXFile included = clang_getIncludedFile(cursor);
  if (included == 0) return;

  CXString includedFileName = clang_getFileName(included);
  printf(" included file: %s\n", clang_getCString(includedFileName));
  clang_disposeString(includedFileName);
}

CXChildVisitResult visitChildrenCallback(CXCursor cursor,
                                         CXCursor parent,
                                         CXClientData client_data) {
  unsigned level = *(unsigned *)client_data;
  printf("  Level: %d\n", level);

  show_spell(cursor);
  show_linkage(cursor);
  show_cursor_kind(cursor);
  show_type(cursor);
  show_parent(cursor, parent);
  show_location(cursor);
  show_usr(cursor);
  show_included_file(cursor);
  printf("\n");

  // visit children recursively
  unsigned next = level + 1;
  clang_visitChildren(cursor, visitChildrenCallback, &next);

  return CXChildVisit_Continue;
}

void show_clang_version(void) {
  CXString version = clang_getClangVersion();
  printf("%s\n", clang_getCString(version));
  clang_disposeString(version);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("ASTVisitor AST_file\n");
    exit(1);
  }

  show_clang_version();

  const auto filename = argv[1];

  // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
  CXIndex index = clang_createIndex(1, 1);

  // load a *.ast file.
  CXTranslationUnit tu = clang_createTranslationUnit(index, filename);
  if (tu == NULL) {
    printf("Cannot create translation unit\n");
    return 1;
  }

  unsigned level = 0;
  CXCursor cursor = clang_getTranslationUnitCursor(tu);
  clang_visitChildren(cursor, visitChildrenCallback, &level);

  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
  return 0;
}
