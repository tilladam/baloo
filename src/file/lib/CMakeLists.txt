set(FILE_LIB_SRCS
    file.cpp
    filefetchjob.cpp
    filemodifyjob.cpp
    filemapping.cpp
    filemonitor.cpp
    taglistjob.cpp
    db.cpp

    indexerconfig.cpp
    ../fileindexerconfig.cpp
    ../regexpcache.cpp
    ../fileexcludefilters.cpp
    xattrdetector.cpp
    filecustommetadata.cpp
)

kde4_add_library(baloofiles SHARED ${FILE_LIB_SRCS})

target_link_libraries(baloofiles
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_SOLID_LIBS}
    ${XAPIAN_LIBRARIES}
    ${QJSON_LIBRARIES}
    baloocore
    balooxapian
)

set_target_properties(baloofiles PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_VERSION_MAJOR}
)

install(TARGETS baloofiles ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    file.h
    filefetchjob.h
    filemodifyjob.h
    filemonitor.h
    taglistjob.h
    indexerconfig.h
    file_export.h

    DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(tests)
add_subdirectory(autotests)
