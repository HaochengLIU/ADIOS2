pybind11_add_module(adios2py MODULE
  ADIOSPy.h
  ADIOSPy.cpp
  adiosPyFunctions.h
  adiosPyFunctions.cpp
  adiosPyTypes.h
  EnginePy.h
  EnginePy.inl
  EnginePy.cpp
  gluePyBind11.cpp
  IOPy.h
  IOPy.cpp
  VariablePy.h
  VariablePy.cpp
)
target_link_libraries(adios2py PRIVATE adios2)
if(ADIOS2_HAVE_MPI)
  target_link_libraries(adios2py PRIVATE PythonModule::mpi4py)
endif()

string(REGEX REPLACE
  "^${PYTHON_PREFIX}/[^/]*/python" "${CMAKE_INSTALL_LIBDIR}/python"
  CMAKE_INSTALL_PYTHONDIR "${PYTHON_SITE_PACKAGES}"
)
set(CMAKE_INSTALL_PYTHONDIR "${CMAKE_INSTALL_PYTHONDIR}"
  CACHE INTERNAL "" FORCE
)
set_target_properties(adios2py PROPERTIES
  OUTPUT_NAME adios2
  LIBRARY_OUTPUT_DIRECTORY ${ADIOS2_BINARY_DIR}/${CMAKE_INSTALL_PYTHONDIR}
  RUNTIME_OUTPUT_DIRECTORY ${ADIOS2_BINARY_DIR}/${CMAKE_INSTALL_PYTHONDIR}
)
install(TARGETS adios2py 
  DESTINATION ${CMAKE_INSTALL_PYTHONDIR}
)
