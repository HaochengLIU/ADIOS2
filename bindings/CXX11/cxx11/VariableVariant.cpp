/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * Variable.h :
 *
 *  Created on: April 16, 2019
 *      Author: Haocheng LIU haocheng.liu@kitware.com
 */
#include "VariableVariant.h"

namespace adios2
{

 VariableVariant::operator bool() const noexcept
 {
    return std::visit([](const auto &var){ return bool(var);}, m_Variables);
 }

void VariableVariant::SetShape(const adios2::Dims &shape)
{
    std::visit([&shape](auto &var){ return var.SetShape(shape);}, m_Variables);
}

void VariableVariant::SetBlockSelection(const size_t blockID)
{
    std::visit([&blockID](auto &var){ return var.SetBlockSelection(blockID);}, m_Variables);
}

void VariableVariant::SetSelection(const adios2::Box<adios2::Dims> &selection)
{
    std::visit([&selection](auto &var){ return var.SetSelection(selection);}, m_Variables);
}

void VariableVariant::SetMemorySelection(const adios2::Box<adios2::Dims> &memorySelection)
{
    std::visit([&memorySelection](auto &var){ return var.SetMemorySelection(memorySelection);}, m_Variables);
}

void VariableVariant::SetStepSelection(const adios2::Box<size_t> &stepSelection)
{
    std::visit([&stepSelection](auto &var){ return var.SetStepSelection(stepSelection);}, m_Variables);
}

size_t VariableVariant::SelectionSize() const
{
    return std::visit([](const auto &var){ return var.SelectionSize();}, m_Variables);
}

std::string VariableVariant::Name() const
{
    return std::visit([](const auto &var){ return var.Name();}, m_Variables);
}

/**
 * Inspects Variable type
 * @return type string literal containing the type: double, float, unsigned
 * int, etc.
 */
std::string VariableVariant::Type() const
{
    return std::visit([](const auto &var){ return var.Type();}, m_Variables);
}

/**
 * Inspects size of the current element type, sizeof(T)
 * @return sizeof(T) for current system
 */
size_t VariableVariant::Sizeof() const
{
    return std::visit([](const auto &var){ return var.Sizeof();}, m_Variables);
}

/**
 * Inspects shape id for current variable
 * @return from enum adios2::ShapeID
 */
adios2::ShapeID VariableVariant::ShapeID() const
{
    return std::visit([](const auto &var){ return var.ShapeID();}, m_Variables);
}

/**
 * Inspects shape in global variables
 * @param step input for a particular Shape if changing over time. If
 * default, either return absolute or in streaming mode it returns the shape
 * for the current engine step
 * @return shape vector
 */
adios2::Dims VariableVariant::Shape(const size_t) const
{
    return std::visit([](const auto &var){ return var.Shape();}, m_Variables);
}

adios2::Dims VariableVariant::Start() const
{
    return std::visit([](const auto &var){ return var.Start();}, m_Variables);
}

adios2::Dims VariableVariant::Count() const
{
    return std::visit([](const auto &var){ return var.Count();}, m_Variables);
}

size_t VariableVariant::Steps() const
{
    return std::visit([](const auto &var){ return var.Steps();}, m_Variables);
}

size_t VariableVariant::StepsStart() const
{
    return std::visit([](const auto &var){ return var.StepsStart();}, m_Variables);
}

size_t VariableVariant::BlockID() const
{
    return std::visit([](const auto &var){ return var.BlockID();}, m_Variables);
}

}
