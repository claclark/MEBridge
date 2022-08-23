building: FORCE
	bazel build -c dbg --cxxopt="-Og" ...

debugging: FORCE
	bazel build -c dbg ...

FORCE:
