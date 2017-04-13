file(GLOB_RECURSE skaffari_SRCS *.cpp *.h)

# Create the application
add_library(skaffari SHARED ${skaffari_SRCS})

pkg_search_module(ICU REQUIRED icu-io)

# Link to Cutelyst
target_link_libraries(skaffari
    Cutelyst::Core
    Cutelyst::View::Grantlee
    Cutelyst::StaticSimple
    Cutelyst::Utils::Validator
    Cutelyst::Utils::Sql
    Cutelyst::Session
    Cutelyst::Authentication
    Cutelyst::StatusMessage
    Cutelyst::Utils::Pagination
    Qt5::Core
    Qt5::Network
    Qt5::Sql
    crypt
    ${ICU_LIBRARIES}
)
