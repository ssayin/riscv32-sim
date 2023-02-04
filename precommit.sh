git diff --name-only | grep "\.hpp$\|\.cpp$" | xargs clang-format -i --verbose
