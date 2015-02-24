install(FILES quotes.frt DESTINATION "${CMAKE_INSTALL_DATADIR}/molecube/")
install(FILES molecube.tmpfiles.conf
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/tmpfiles.d/"
  RENAME molecube.conf)
