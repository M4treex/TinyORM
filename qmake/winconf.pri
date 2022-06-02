# WinApi
# ---

# All have to be defined because of checks at the beginning of the qt_windows.h
# Windows 10 1903 "19H1" - 0x0A000007
DEFINES += WINVER=_WIN32_WINNT_WIN10
DEFINES += NTDDI_VERSION=NTDDI_WIN10_19H1
DEFINES += _WIN32_WINNT=_WIN32_WINNT_WIN10
# Internet Explorer 11
DEFINES += _WIN32_IE=_WIN32_IE_IE110
# Exclude unneeded header files
DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += NOMINMAX

# Compiler and Linker options
# ---

win32-msvc {
    # I don't use -MP flag, because using jom
    # strict-c++ does not enable -permissive- on MSVC
    QMAKE_CXXFLAGS += -guard:cf -bigobj
    QMAKE_CXXFLAGS_DEBUG += -RTC1
    QMAKE_CXXFLAGS_WARN_ON = -external:anglebrackets -external:W0 -WX -W4 -wd4702
    QMAKE_LFLAGS += /guard:cf /WX
    QMAKE_LFLAGS_RELEASE += /OPT:REF,ICF=5

    # Latest qmake's msvc fixes
    greaterThan(QMAKE_MSC_VER, 1909) {
        QMAKE_CXXFLAGS     += -permissive-
        QMAKE_CXXFLAGS     -= -Zc:referenceBinding
    }

    greaterThan(QMAKE_MSC_VER, 1919) {
        QMAKE_CXXFLAGS     += -Zc:externConstexpr
    }

    greaterThan(QMAKE_MSC_VER, 1927) {
        # Visual Studio 2019 (16.8 or 16.9) / Visual C++ 19.28 and up
        MSVC_VER            = 16.8
        QMAKE_CFLAGS_C11    = /std:c11
        QMAKE_CFLAGS_C17    = /std:c17
    }

    greaterThan(QMAKE_MSC_VER, 1928) {
        # Visual Studio 2019 (16.10 or 16.11) / Visual C++ 19.29 and up
        MSVC_VER            = 16.10

        # -std:c++20 compiler option for Visual Studio 2019 16.11.0 and up
        greaterThan(QMAKE_MSC_FULL_VER, 192930132): QMAKE_CXXFLAGS_CXX2A = -std:c++20
    }

    greaterThan(QMAKE_MSC_VER, 1929) {
        # Visual Studio 2022 (17.0) / Visual C++ 19.30 and up
        MSVC_VER            = 17.0
    }
}

win32-clang-g++ {
    # -mthreads is unused on Clang
    QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
    QMAKE_LFLAGS_EXCEPTIONS_ON -= -mthreads
}
