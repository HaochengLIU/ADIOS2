/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Variable.h :
 *
 *  Created on: April 16, 2019
 *      Author: Haocheng LIU haocheng.liu@kitware.com
 */

#ifndef ADIOS2_BINDINGS_CXX11_CXX11_VARIABLEVARIANT_H_
#define ADIOS2_BINDINGS_CXX11_CXX11_VARIABLEVARIANT_H_

#include "Operator.h"
#include "Variable.h"

#include "adios2/ADIOSTypes.h"

#include <variant>

namespace adios2
{

namespace detail
{
template<typename... Types>
struct TypesToVariantOfVariables : std::false_type {};

template<template<typename...> class Container, typename... Types>
struct TypesToVariantOfVariables<Container<Types...>> : std::true_type
{
    using type = std::variant<Variable<Types>...>;
};
}

/// \cond EXCLUDE_FROM_DOXYGEN
// forward declare
class IO;     // friend
class Engine; // friend

/** A class which can be used as a convenient way to
 *  define/inquire a template independent variable. It can be helpful
 *  when reading adios2 data if you do not know the type at compile time as it
 *  exposes all template independent APIs of Variable class.
 *  It's a workaround of the fact that type info is stored as a string in adios2
 *  and C++ prohibits differentiating function by return type.
 *  It should not be used directly without IO.
 */
class VariableVariant
{
public:
    using VariantOfVars = detail::TypesToVariantOfVariables<detail::CommonTypes::list>::type;

    friend class IO;
    friend class Engine;
    /**
     * Empty (default) constructor, use it as a placeholder for future
     * variables from IO:DefineVariable<T> or IO:InquireVariable<T>.
     * Can be used with STL containers.
     */
    VariableVariant() = default;

    template<typename T>
    VariableVariant(const Variable<T> & var): m_Variables(var)
    {
    }

    /** Default, using RAII STL containers */
    ~VariableVariant() = default;

    template<typename T>
    void SetVariable(const Variable<T>& var);

    /** Checks if object is valid, e.g. if( variable ) { //..valid } */
    virtual explicit operator bool() const noexcept;

    /**
     * Set new shape, care must be taken when reading back the variable for
     * different steps. Only applies to Global arrays.
     * @param shape new shape dimensions array
     */
    virtual void SetShape(const adios2::Dims &shape);

    /**
     * Read mode only. Required for reading local variables, ShapeID() =
     * ShapeID::LocalArray or ShapeID::LocalValue. For Global Arrays it will Set
     * the appropriate Start and Count Selection for the global array
     * coordinates.
     * @param blockID: variable block index defined at write time. Blocks can be
     * inspected with bpls -D variableName
     */
    virtual void SetBlockSelection(const size_t blockID);

    /**
     * Sets a variable selection modifying current {start, count}
     * Count is the dimension from Start point
     * @param selection input {start, count}
     */
    virtual void SetSelection(const adios2::Box<adios2::Dims> &selection);

    /**
     * Set the local start (offset) point to the memory pointer passed at Put
     * and the memory local dimensions (count). Used for non-contiguous memory
     * writes and reads (e.g. multidimensional ghost-cells).
     * Currently not working for calls to Get.
     * @param memorySelection {memoryStart, memoryCount}
     * <pre>
     * 		memoryStart: relative local offset of variable.start to the
     * contiguous memory pointer passed at Put from which data starts. e.g. if
     * variable.Start() = {rank*Ny,0} and there is 1 ghost cell per dimension,
     * then memoryStart = {1,1}
     * 		memoryCount: local dimensions for the contiguous memory pointer
     * passed at Put, e.g. if there is 1 ghost cell per dimension and
     * variable.Count() = {Ny,Nx}, then memoryCount = {Ny+2,Nx+2}
     * </pre>
     */
    virtual void SetMemorySelection(const adios2::Box<adios2::Dims> &memorySelection);

    /**
     * Sets a step selection modifying current startStep, countStep
     * countStep is the number of steps from startStep point
     * @param stepSelection input {startStep, countStep}
     */
    virtual void SetStepSelection(const adios2::Box<size_t> &stepSelection);

    /**
     * Returns the number of elements required for pre-allocation based on
     * current count and stepsCount
     * @return elements of type T required for pre-allocation
     */
    virtual size_t SelectionSize() const;

    /**
     * Inspects Variable name
     * @return name
     */
    virtual std::string Name() const;

    /**
     * Inspects Variable type
     * @return type string literal containing the type: double, float, unsigned
     * int, etc.
     */
    virtual std::string Type() const;

    /**
     * Inspects size of the current element type, sizeof(T)
     * @return sizeof(T) for current system
     */
    virtual size_t Sizeof() const;

    /**
     * Inspects shape id for current variable
     * @return from enum adios2::ShapeID
     */
    virtual adios2::ShapeID ShapeID() const;

    /**
     * Inspects shape in global variables
     * @param step input for a particular Shape if changing over time. If
     * default, either return absolute or in streaming mode it returns the shape
     * for the current engine step
     * @return shape vector
     */
    virtual adios2::Dims Shape(const size_t step = adios2::EngineCurrentStep) const;

    /**
     * Inspects current start point
     * @return start vector
     */
    virtual adios2::Dims Start() const;

    /**
     * Inspects current count from start
     * @return count vector
     */
    virtual adios2::Dims Count() const;

    /**
     * For read mode, inspect the number of available steps
     * @return available steps
     */
    virtual size_t Steps() const;

    /**
     * For read mode, inspect the start step for available steps
     * @return available start step
     */
    virtual size_t StepsStart() const;

    /**
     * For read mode, retrieve current BlockID, default = 0 if not set with
     * SetBlockID
     * @return current block id
     */
    virtual size_t BlockID() const;

protected:
    VariantOfVars  m_Variables;
};

template<typename T>
void VariableVariant::SetVariable(const Variable<T> &var)
{
  this->m_Variables = var;
}

}

#endif
