set (AMREX_DEMO_EXEC_FILES "")

list (APPEND AMREX_DEMO_EXEC_FILES
    main.cpp
)

foreach( execsource ${AMREX_DEMO_EXEC_FILES} )
    get_filename_component( execname ${execsource} NAME_WE )
    add_executable( ${execname} ${execsource} )
    target_link_libraries( ${execname} AMREX_DEMO )
    install( TARGETS ${execname} DESTINATION bin )
endforeach( execsource ${AMREX_DEMO_EXEC_FILES} )
