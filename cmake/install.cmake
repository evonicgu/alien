CPMAddPackage(
        NAME cxxopts
        GITHUB_REPOSITORY jarro2783/cxxopts
        VERSION 2.2.1
)

CPMAddPackage(
        NAME utf8proc
        GITHUB_REPOSITORY JuliaStrings/utf8proc
        VERSION 2.6.1
)

set(JSON_Install on)

CPMAddPackage("gh:nlohmann/json@3.10.4")

set(INJA_USE_EMBEDDED_JSON off)

CPMAddPackage(
        NAME inja
        GITHUB_REPOSITORY pantor/inja
        VERSION 3.3.0
)