if (LRELEASE_CMD)

    foreach(l ${LANGS})
        execute_process(COMMAND ${LRELEASE_CMD} -silent ${CMAKE_CURRENT_SOURCE_DIR}/skaffari_${l}.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/skaffari_${l}.qm)
        list(APPEND QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/skaffari_${l}.qm)
        
        execute_process(COMMAND ${LRELEASE_CMD} -silent ${CMAKE_CURRENT_SOURCE_DIR}/skaffaricmd_${l}.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/skaffaricmd_${l}.qm)
        list(APPEND QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/skaffaricmd_${l}.qm)
    endforeach(l)

    install (FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_LOCALEDIR})

endif(LRELEASE_CMD)
 
