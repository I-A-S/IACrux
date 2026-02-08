include(FetchContent)

FetchContent_Declare(
    Auxid
    GIT_REPOSITORY https://github.com/I-A-S/Auxid
    GIT_TAG        main
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    IATest
    GIT_REPOSITORY https://github.com/I-A-S/IATest
    GIT_TAG        main
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    unordered_dense
    GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
    GIT_TAG        v4.8.1
    SYSTEM
)

FetchContent_MakeAvailable(Auxid IATest unordered_dense)
