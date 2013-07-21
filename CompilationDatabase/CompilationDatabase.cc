#include <cstdio>
#include <cstdlib>
#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>

void show_clang_version(void) {
  CXString version = clang_getClangVersion();
  printf("%s\n", clang_getCString(version));
  clang_disposeString(version);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("CompilationDatabase directory\n");
    exit(1);
  }

  show_clang_version();

  CXCompilationDatabase_Error error;
  CXCompilationDatabase db = clang_CompilationDatabase_fromDirectory(argv[1], &error);
  switch (error) {
    case CXCompilationDatabase_NoError:
      break;
    case CXCompilationDatabase_CanNotLoadDatabase:
      printf("Cannot load database\n");
      exit(1);
    default: 
      printf("unknown return\n");
      exit(1);
  }

  CXCompileCommands cmds = clang_CompilationDatabase_getAllCompileCommands(db);
  auto numCmds = clang_CompileCommands_getSize(cmds);
  printf("CommandNum: %d\n", numCmds);

  for (auto i = 0U; i < numCmds; i++) {
    CXCompileCommands cmd = clang_CompileCommands_getCommand(cmds, i);
    auto numArgs = clang_CompileCommand_getNumArgs(cmd);
    printf(" CommandArgs: %d\n", numArgs);
    for (auto j = 0U; j < numArgs; j++) {
      CXString arg = clang_CompileCommand_getArg(cmd, j);
      printf("  %s \n", clang_getCString(arg));
      clang_disposeString(arg);
    }
  }

  clang_CompileCommands_dispose(cmds);
  clang_CompilationDatabase_dispose(db);
  return 0;
}
