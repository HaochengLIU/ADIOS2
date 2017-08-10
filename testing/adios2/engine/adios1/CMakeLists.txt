#------------------------------------------------------------------------------#
# Distributed under the OSI-approved Apache License, Version 2.0.  See
# accompanying file Copyright.txt for details.
#------------------------------------------------------------------------------#

find_package(ADIOS1 COMPONENTS sequential REQUIRED)

add_executable(TestADIOS1WriteRead TestADIOS1WriteRead.cpp)
target_link_libraries(TestADIOS1WriteRead adios2 gtest adios1::adios)

if(NOT ADIOS2_HAVE_MPI)
  gtest_add_tests(TARGET TestADIOS1WriteRead)
else()
  # MPI versions of the test
  message("MPIEXEC: ${MPIEXEC}")
  message("MPIEXEC_NUMPROC_FLAG: ${MPIEXEC_NUMPROC_FLAG}")
  message("MPIEXEC_MAX_NUMPROCS: ${MPIEXEC_MAX_NUMPROCS}")

  message ("Running MPI version of TestADIOS1WriteRead")
  gtest_add_tests(TARGET TestADIOS1WriteRead EXEC_WRAPPER ${MPIEXEC}
                                                        ${MPIEXEC_NUMPROC_FLAG}
                                                        2)
endif()
#endif()
