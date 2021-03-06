#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

if(ADIOS2_HAVE_MPI)
  add_executable(hello_bpWriter helloBPWriter.cpp)
  target_include_directories(hello_bpWriter PRIVATE ${MPI_C_INCLUDE_PATH})
  target_link_libraries(hello_bpWriter ${MPI_C_LIBRARIES})
  
  add_executable(hello_bpWriter_c helloBPWriter.c)
  target_include_directories(hello_bpWriter_c PRIVATE ${MPI_C_INCLUDE_PATH})
  target_link_libraries(hello_bpWriter_c ${MPI_C_LIBRARIES})
  
  if(ADIOS2_HAVE_Fortran)
    add_executable(hello_bpWriter_f helloBPWriter.f90)
    target_include_directories(hello_bpWriter_f 
                               PRIVATE ${MPI_Fortran_INCLUDE_PATH} 
                                       ${MPI_C_INCLUDE_PATH})
                                               
    target_link_libraries(hello_bpWriter_f PRIVATE ${MPI_Fortran_LIBRARIES} 
                                                   ${MPI_C_LIBRARIES})
  endif()
  
else()
  add_executable(hello_bpWriter helloBPWriter_nompi.cpp)
  add_executable(hello_bpWriter_c helloBPWriter_nompi.c)

  if(ADIOS2_HAVE_Fortran)
    add_executable(hello_bpWriter_f helloBPWriter_nompi.f90)
  endif()
  
endif()

target_link_libraries(hello_bpWriter adios2)
target_link_libraries(hello_bpWriter_c adios2)

if(ADIOS2_HAVE_Fortran)
  target_link_libraries(hello_bpWriter_f PRIVATE adios2_f)
endif()
