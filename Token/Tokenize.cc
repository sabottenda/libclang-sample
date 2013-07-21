#include <cstdio>
#include <cstdlib>
#include <clang-c/Index.h>

const char *_getTokenKindSpelling(CXTokenKind kind) {
  switch (kind) {
    case CXToken_Punctuation: return "Punctuation"; break;
    case CXToken_Keyword:     return "Keyword"; break;
    case CXToken_Identifier:  return "Identifier"; break;
    case CXToken_Literal:     return "Literal"; break;
    case CXToken_Comment:     return "Comment"; break;
    default:                  return "Unknown"; break;
  }
}

void show_all_tokens(const CXTranslationUnit &tu, const CXToken *tokens, unsigned numTokens) {
  printf("=== show tokens ===\n");
  printf("NumTokens: %d\n", numTokens);
  for (auto i = 0U; i < numTokens; i++) {
    const CXToken &token = tokens[i];
    CXTokenKind kind = clang_getTokenKind(token);
    CXString spell = clang_getTokenSpelling(tu, token);
    CXSourceLocation loc = clang_getTokenLocation(tu, token);

    CXFile file;
    unsigned line, column, offset;
    clang_getFileLocation(loc, &file, &line, &column, &offset);
    CXString fileName = clang_getFileName(file);

    printf("Token: %d\n", i);
    printf(" Text: %s\n", clang_getCString(spell));
    printf(" Kind: %s\n", _getTokenKindSpelling(kind));
    printf(" Location: %s:%d:%d:%d\n",
           clang_getCString(fileName), line, column, offset);
    printf("\n");

    clang_disposeString(fileName);
    clang_disposeString(spell);
  }
}

unsigned get_filesize(const char *fileName) {
  FILE *fp = fopen(fileName, "r");
  fseek(fp, 0, SEEK_END);
  auto size = ftell(fp);
  fclose(fp);
  return size;
}

CXSourceRange get_filerange(const CXTranslationUnit &tu, const char *filename) {
  CXFile file = clang_getFile(tu, filename);
  auto fileSize = get_filesize(filename);

  // get top/last location of the file
  CXSourceLocation topLoc  = clang_getLocationForOffset(tu, file, 0);
  CXSourceLocation lastLoc = clang_getLocationForOffset(tu, file, fileSize);
  if (clang_equalLocations(topLoc,  clang_getNullLocation()) ||
      clang_equalLocations(lastLoc, clang_getNullLocation()) ) {
    printf("cannot retrieve location\n");
    exit(1);
  }

  // make a range from locations
  CXSourceRange range = clang_getRange(topLoc, lastLoc);
  if (clang_Range_isNull(range)) {
    printf("cannot retrieve range\n");
    exit(1);
  }

  return range;
}

void show_clang_version(void) {
  CXString version = clang_getClangVersion();
  printf("%s\n", clang_getCString(version));
  clang_disposeString(version);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Tokenize filename [options ...]\n");
    exit(1);
  }

  show_clang_version();

  const auto filename = argv[1];
  const auto cmdArgs = &argv[2];
  auto numArgs = argc - 2;
  
  // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
  CXIndex index = clang_createIndex(1, 1);

  // create Translation Unit
  CXTranslationUnit tu = clang_parseTranslationUnit(index, filename, cmdArgs, numArgs, NULL, 0, 0);
  if (tu == NULL) {
    printf("Cannot parse translation unit\n");
    return 1;
  }

  // get CXSouceRange of the file
  CXSourceRange range = get_filerange(tu, filename);

  // tokenize in the range
  CXToken *tokens;
  unsigned numTokens;
  clang_tokenize(tu, range, &tokens, &numTokens);

  // show tokens
  show_all_tokens(tu, tokens, numTokens);

  clang_disposeTokens(tu, tokens, numTokens);
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
  return 0;
}
