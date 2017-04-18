set(skaffari_SRCS
    imap/skaffariimap.cpp
    imap/skaffariimap.h
    imap/skaffariimaperror.cpp
    imap/skaffariimaperror.h
    imap/skaffariimaperror_p.h
    objects/account.cpp
    objects/account.h
    objects/account_p.h
    objects/adminaccount.cpp
    objects/adminaccount.h
    objects/adminaccount_p.h
    objects/domain.cpp
    objects/domain.h
    objects/domain_p.h
    objects/simpleadmin.cpp
    objects/simpleadmin.h
    objects/simpleadmin_p.h
    objects/simpledomain.cpp
    objects/simpledomain.h
    objects/simpledomain_p.h
    objects/skaffarierror.cpp
    objects/skaffarierror.h
    objects/skaffarierror_p.h
    utils/language.cpp
    utils/language.h
    utils/language_p.h
    utils/timeutils.cpp
    utils/timeutils.h
    accounteditor.cpp
    accounteditor.h
    admineditor.cpp
    admineditor.h
    authstoresql.cpp
    authstoresql.h
    domaineditor.cpp
    domaineditor.h
    login.cpp
    login.h
    logout.cpp
    logout.h
    myaccount.cpp
    myaccount.h
    root.cpp
    root.h
    sengine.cpp
    sengine.h
    skaffari.cpp
    skaffari.h
    skaffariengine.cpp
    skaffariengine.h
    skaffariengine_p.h
    ../common/password.cpp
    ../common/password.h
)

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
