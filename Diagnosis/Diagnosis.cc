#include <cstdio>
#include <cstdlib>
#include <clang-c/Index.h>

void show_diagnosis_format(const CXDiagnostic &diag) {
  unsigned formatOption = CXDiagnostic_DisplaySourceLocation |
                          CXDiagnostic_DisplayColumn |
                          CXDiagnostic_DisplaySourceRanges |
                          CXDiagnostic_DisplayOption |
                          CXDiagnostic_DisplayCategoryId|
                          CXDiagnostic_DisplayCategoryName;
  CXString format = clang_formatDiagnostic(diag, formatOption);
  printf("  Format: %s\n", clang_getCString(format));
  clang_disposeString(format);
}

void show_diagnosis_severity(const CXDiagnostic &diag) {
  CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);
  const char *severityText;
  switch (severity) {
    case CXDiagnostic_Ignored: severityText = "Ignored"; break;
    case CXDiagnostic_Note:    severityText = "Note";    break;
    case CXDiagnostic_Warning: severityText = "Warning"; break;
    case CXDiagnostic_Error:   severityText = "Error";   break;
    case CXDiagnostic_Fatal:   severityText = "Fatal";   break;
    default:                   severityText = "Unknown"; break;
  }
  printf("  Severity: %s\n", severityText);
}

void show_diagnosis_location(const CXDiagnostic &diag) {
  CXSourceLocation loc = clang_getDiagnosticLocation(diag);
  CXFile file;
  unsigned line, column, offset;
  clang_getSpellingLocation(loc, &file, &line, &column, &offset);
  CXString fileText = clang_getFileName(file);
  printf("  Location: %s:%d:%d:%d\n", 
         clang_getCString(fileText), line, column, offset);
  clang_disposeString(fileText);
}

void show_diagnosis_category(const CXDiagnostic &diag) {
  CXString catText = clang_getDiagnosticCategoryText(diag);
  printf("  Category: %s\n", clang_getCString(catText));
  clang_disposeString(catText);
}

void show_diagnosis_range(const CXDiagnostic &diag) {
  CXFile file;
  unsigned line, column, offset;
  CXString fileText;

  unsigned numRange = clang_getDiagnosticNumRanges(diag);
  printf("  NumRange: %d\n", numRange);
  for (auto j = 0U; j < numRange; j++) {
    CXSourceRange range = clang_getDiagnosticRange(diag, j);
    printf("    Range %d\n", j);

    CXSourceLocation start = clang_getRangeStart(range);
    clang_getSpellingLocation(start, &file, &line, &column, &offset);
    fileText =clang_getFileName(file);
    printf("      Start: %s:%d:%d:%d\n",
           clang_getCString(fileText), line, column, offset);
    clang_disposeString(fileText);

    CXSourceLocation end = clang_getRangeEnd(range);
    clang_getSpellingLocation(end, &file, &line, &column, &offset);
    fileText =clang_getFileName(file);
    printf("      End: %s:%d:%d:%d\n",
           clang_getCString(fileText), line, column, offset);
    clang_disposeString(fileText);
  }
}

void show_diagnosis_fixit(const CXDiagnostic &diag) {
  unsigned numFixit = clang_getDiagnosticNumFixIts(diag);
  printf("  NumFixit: %d\n", numFixit);
  for (auto j = 0U; j < numFixit; j++) {
    CXString fixit = clang_getDiagnosticFixIt(diag, j, NULL);
    printf("    Fixit: %s\n", clang_getCString(fixit));
    clang_disposeString(fixit);
  }
}

void show_diagnosis_child(const CXDiagnostic &diag) {
  CXDiagnosticSet childDiagSet = clang_getChildDiagnostics(diag);
  unsigned numChildDiag = clang_getNumDiagnosticsInSet(childDiagSet);
  printf("  NumChildDiag: %d\n", numChildDiag);

  // TODO: show child DiagnosticSet recursively(?)
}

void diagnosis(const CXTranslationUnit &tu) {
  CXDiagnosticSet diagSet = clang_getDiagnosticSetFromTU(tu);
  unsigned numDiag = clang_getNumDiagnosticsInSet(diagSet);
  printf("numDiag: %d\n", numDiag);

  for (auto i = 0U; i < numDiag; i++) {
    CXDiagnostic diag = clang_getDiagnosticInSet(diagSet, i);

    // show diagnosis spell
    CXString diagText = clang_getDiagnosticSpelling(diag);
    printf("  Diagnosis: %s\n", clang_getCString(diagText));
    clang_disposeString(diagText);

    show_diagnosis_format(diag);   // format
    show_diagnosis_severity(diag); // severity
    show_diagnosis_location(diag); // location
    show_diagnosis_category(diag); // category
    show_diagnosis_range(diag);    // range
    show_diagnosis_fixit(diag);    // fixit
    show_diagnosis_child(diag);    // child
    printf("\n");

    clang_disposeDiagnostic(diag);
  }
  clang_disposeDiagnosticSet(diagSet);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Diagnosis filename [options ...]\n");
    exit(1);
  }

  const auto filename = argv[1];
  const auto cmdArgs = &argv[2];
  auto numArgs = argc - 2;

  // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
  CXIndex index = clang_createIndex(1, 0);

  // create Translation Unit
  CXTranslationUnit tu = clang_parseTranslationUnit(index, filename, cmdArgs, numArgs,
                                                    NULL, 0, 0);
  if (tu == NULL) {
    printf("Cannot parse translation unit\n");
    return 1;
  }

  // show diagnosis
  diagnosis(tu);

  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
  return 0;
}
