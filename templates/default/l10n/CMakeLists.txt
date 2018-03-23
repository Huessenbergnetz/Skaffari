if (LRELEASE_CMD)

    set(TMPL_NAME "default")

    foreach(l ${LANGS})
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tmpl_${TMPL_NAME}_${l}.ts")
            execute_process(COMMAND ${LRELEASE_CMD} -silent ${CMAKE_CURRENT_SOURCE_DIR}/tmpl_${TMPL_NAME}_${l}.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/${TMPL_NAME}_${l}.qm)
            list(APPEND QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/${TMPL_NAME}_${l}.qm)
        else(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tmpl_${TMPL_NAME}_${l}.ts")
            message(WARNING "Can not find ${CMAKE_CURRENT_SOURCE_DIR}/tmpl_${TMPL_NAME}_${l}.ts")
        endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tmpl_${TMPL_NAME}_${l}.ts")
    endforeach(l)

    install (FILES ${QM_FILES} DESTINATION ${TEMPLATES_INSTALL_DIR}/${TMPL_NAME}/l10n)

endif(LRELEASE_CMD)
