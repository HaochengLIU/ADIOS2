#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

# MPI versions of the test are not properly implemented at the moment
if(ADIOS2_HAVE_ADIOS1)
  find_package(ADIOS1 COMPONENTS sequential REQUIRED)

  add_executable(TestBPWriteRead TestBPWriteRead.cpp)
  target_link_libraries(TestBPWriteRead adios2 gtest adios1::adios)

  #add_executable(TestBPWriteReadstdio TestBPWriteReadstdio.cpp)
  #target_link_libraries(TestBPWriteReadstdio adios2 gtest adios1::adios)

  #add_executable(TestBPWriteReadfstream TestBPWriteReadfstream.cpp)
  #target_link_libraries(TestBPWriteReadfstream adios2 gtest adios1::adios)
  if(NOT ADIOS2_HAVE_MPI)
    gtest_add_tests(TARGET TestBPWriteRead)
    gtest_add_tests(TARGET TestBPWriteReadstdio)
    gtest_add_tests(TARGET TestBPWriteReadfstream)
  else()
    message ("ADIOS2_HAVE_MPI")
    gtest_add_tests(TARGET TestBPWriteRead EXEC_WRAPPER ${MPIEXEC}
                                                        ${MPIEXEC_NUMPROC_FLAG}
                                                        2)
                                                        #        ${MPIEXEC_MAX_NUMPROCS})
    #gtest_add_tests(TARGET TestBPWriteReadstdio)
    #gtest_add_tests(TARGET TestBPWriteReadfstream)
  endif()
endif()

#add_executable(TestBPWriteProfilingJSON TestBPWriteProfilingJSON.cpp)
#target_link_libraries(TestBPWriteProfilingJSON adios2 gtest gtest_main NLohmannJson)

if(NOT ADIOS2_HAVE_MPI)
  gtest_add_tests(TARGET TestBPWriteProfilingJSON)
else()
  #gtest_add_tests(TARGET TestBPWriteProfilingJSON)
endif()
