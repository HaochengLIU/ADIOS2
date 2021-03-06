#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

if(ADIOS2_HAVE_MPI)
  add_executable(heatTransfer_write_adios2
    main.cpp
    HeatTransfer.cpp
    Settings.cpp
    IO_adios2.cpp
  )
  target_include_directories(heatTransfer_write_adios2
    PRIVATE ${MPI_C_INCLUDE_PATH}
  )
  target_link_libraries(heatTransfer_write_adios2
    adios2 ${MPI_C_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
  )
  target_compile_definitions(heatTransfer_write_adios2 PRIVATE
   -DDEFAULT_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/config.xml
  )

  if(ADIOS2_HAVE_ADIOS1)
    add_executable(heatTransfer_write_adios1
      main.cpp
      HeatTransfer.cpp
      Settings.cpp
      IO_adios1.cpp
    )
    target_include_directories(heatTransfer_write_adios1
      PRIVATE ${MPI_C_INCLUDE_PATH}
    )
    target_link_libraries(heatTransfer_write_adios1
      adios1::adios ${MPI_C_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    )
  endif()

  if(ADIOS2_HAVE_HDF5)
    add_executable(heatTransfer_write_hdf5
      main.cpp
      HeatTransfer.cpp
      Settings.cpp
      IO_hdf5_a.cpp
    )
    target_include_directories(heatTransfer_write_hdf5
      PRIVATE ${MPI_C_INCLUDE_PATH} ${HDF5_C_INCLUDE_DIRS}
    )
    target_link_libraries(heatTransfer_write_hdf5
      ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(heatTransfer_write_ph5
      main.cpp
      HeatTransfer.cpp
      Settings.cpp
      IO_ph5.cpp
    )
    target_include_directories(heatTransfer_write_ph5
      PRIVATE ${MPI_C_INCLUDE_PATH} ${HDF5_C_INCLUDE_DIRS}
    )
    target_link_libraries(heatTransfer_write_ph5
      ${HDF5_C_LIBRARIES} ${MPI_C_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(heatTransfer_write_a2h5
      main.cpp
      HeatTransfer.cpp
      Settings.cpp
      IO_ph5_adios2.cpp
    )

    target_include_directories(heatTransfer_write_a2h5
      PRIVATE ${MPI_C_INCLUDE_PATH}
    )
    target_link_libraries(heatTransfer_write_a2h5
      adios2 ${MPI_C_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    )
  endif()
endif()
