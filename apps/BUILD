cc_binary(
    name = "bridge",
    srcs = [
        "bridge.cc",
    ],
    deps = [
      "//lib:mars",
    ],
    copts = [
        "'-std=c++14'",
    ],
    linkopts = [
        "-pthread",
    ],
)

cc_binary(
    name = "echo",
    srcs = [
        "echo.cc",
    ],
    deps = [
      "//lib:mars",
    ],
    copts = [
        "'-std=c++14'",
    ],
    linkopts = [
        "-pthread",
    ],
)
