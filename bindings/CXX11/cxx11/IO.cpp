/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * IO.cpp :
 *
 *  Created on: Jun 4, 2018
 *      Author: William F Godoy godoywf@ornl.gov
 */

#include "IO.h"
#include "IO.tcc"

#include "adios2/ADIOSMPI.h"
#include "adios2/core/IO.h"

namespace adios2
{

IO::operator bool() const noexcept { return (m_IO == nullptr) ? false : true; }

bool IO::InConfigFile() const
{
    helper::CheckForNullptr(m_IO, "in call to IO::InConfigFile");
    return m_IO->InConfigFile();
}

void IO::SetEngine(const std::string engineType)
{
    helper::CheckForNullptr(m_IO, "in call to IO::SetEngine");
    m_IO->SetEngine(engineType);
}

void IO::SetParameter(const std::string key, const std::string value)
{
    helper::CheckForNullptr(m_IO, "in call to IO::SetParameter");
    m_IO->SetParameter(key, value);
}

void IO::SetParameters(const Params &parameters)
{
    helper::CheckForNullptr(m_IO, "in call to IO::SetParameters");
    m_IO->SetParameters(parameters);
}

Params IO::Parameters() const
{
    helper::CheckForNullptr(m_IO, "in call to IO:::Parameters");
    return m_IO->m_Parameters;
}

size_t IO::AddTransport(const std::string type, const Params &parameters)
{
    helper::CheckForNullptr(m_IO, "in call to IO::AddTransport");
    return m_IO->AddTransport(type, parameters);
}

void IO::SetTransportParameter(const size_t transportIndex,
                               const std::string key, const std::string value)
{
    helper::CheckForNullptr(m_IO, "in call to IO::SetTransportParameter");
    m_IO->SetTransportParameter(transportIndex, key, value);
}

#if __cplusplus > 201402L
VariableVariant IO::InquireVariable(const std::string &name, const std::string &type)
{
    VariableVariant var;
    if (type == "char")
    {
      var = this->InquireVariable<char>(name);
    }
    else if (type == "int")
    {
      var = this->InquireVariable<int>(name);
    }
    if (type == "float")
    {
      var = this->InquireVariable<float>(name);
    }
    else if (type == "double")
    {
      var = this->InquireVariable<double>(name);
    }
    if (type == "float complex")
    {
      var = this->InquireVariable<std::complex<float>>(name);
    }
    else if (type == "double complex")
    {
      var = this->InquireVariable<std::complex<double>>(name);
    }
    if (type == "signed char")
    {
      var = this->InquireVariable<signed char>(name);
    }
    else if (type == "short")
    {
      var = this->InquireVariable<short>(name);
    }
    else if (type == "long int")
    {
      var = this->InquireVariable<long int>(name);
    }
    else if (type == "long long int")
    {
      var = this->InquireVariable<long long int>(name);
    }
    else if (type == "string")
    {
      var = this->InquireVariable<std::string>(name);
    }
    else if (type == "string array")
    {
      var = this->InquireVariable<int64_t>(name);
    }
    else if (type == "unsigned char")
    {
      var = this->InquireVariable<int8_t>(name);
    }
    else if (type == "unsigned short")
    {
      var = this->InquireVariable<int64_t>(name);
    }
    else if (type == "unsigned int")
    {
      var = this->InquireVariable<int8_t>(name);
    }
    else if (type == "unsigned long int")
    {
      var = this->InquireVariable<int8_t>(name);
    }
    else if (type == "unsigned long long int")
    {
      var = this->InquireVariable<int8_t>(name);
    }
    return var;
}
#endif

bool IO::RemoveVariable(const std::string &name)
{
    helper::CheckForNullptr(m_IO, "in call to IO::RemoveVariable");
    return m_IO->RemoveVariable(name);
}

void IO::RemoveAllVariables()
{
    helper::CheckForNullptr(m_IO, "in call to IO::RemoveAllVariables");
    m_IO->RemoveAllVariables();
}

bool IO::RemoveAttribute(const std::string &name)
{
    helper::CheckForNullptr(m_IO, "in call to IO::RemoveAttribute");
    return m_IO->RemoveAttribute(name);
}

void IO::RemoveAllAttributes()
{
    helper::CheckForNullptr(m_IO, "in call to IO::RemoveAllAttributes");
    m_IO->RemoveAllAttributes();
}

#ifdef ADIOS2_HAVE_MPI
Engine IO::Open(const std::string &name, const Mode mode, MPI_Comm comm)
{
    helper::CheckForNullptr(m_IO,
                            "for engine " + name + ", in call to IO::Open");
    return Engine(&m_IO->Open(name, mode, comm));
}
#endif

Engine IO::Open(const std::string &name, const Mode mode)
{
    helper::CheckForNullptr(m_IO,
                            "for engine " + name + ", in call to IO::Open");
    return Engine(&m_IO->Open(name, mode));
}

void IO::FlushAll()
{
    helper::CheckForNullptr(m_IO, "in call to IO::FlushAll");
    m_IO->FlushAll();
}

void IO::LockDefinitions()
{
    helper::CheckForNullptr(m_IO, "in call to IO::LockDefinitions");
    m_IO->LockDefinitions();
}

std::map<std::string, Params> IO::AvailableVariables()
{
    helper::CheckForNullptr(m_IO, "in call to IO::AvailableVariables");
    return m_IO->GetAvailableVariables();
}

std::map<std::string, Params>
IO::AvailableAttributes(const std::string &variableName,
                        const std::string separator)
{
    helper::CheckForNullptr(m_IO, "in call to IO::AvailableAttributes");
    return m_IO->GetAvailableAttributes(variableName, separator);
}

std::string IO::VariableType(const std::string &name) const
{
    helper::CheckForNullptr(m_IO, "in call to IO::VariableType");
    return m_IO->InquireVariableType(name);
}

std::string IO::AttributeType(const std::string &name) const
{
    helper::CheckForNullptr(m_IO, "in call to IO::AttributeType");
    return m_IO->InquireAttributeType(name);
}

size_t IO::AddOperation(const Operator op, const Params &parameters)
{
    helper::CheckForNullptr(m_IO, "in call to IO::AddOperation");
    return m_IO->AddOperation(*op.m_Operator, parameters);
}

std::string IO::EngineType() const
{
    helper::CheckForNullptr(m_IO, "in call to IO::EngineType");
    return m_IO->m_EngineType;
}

// PRIVATE
IO::IO(core::IO *io) : m_IO(io) {}

// Explicit declaration of the public template methods
// Limits the types
#define declare_template_instantiation(T)                                      \
    template Variable<T> IO::DefineVariable(const std::string &, const Dims &, \
                                            const Dims &, const Dims &,        \
                                            const bool);                       \
                                                                               \
    template Variable<T> IO::InquireVariable<T>(const std::string &);

ADIOS2_FOREACH_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

#define declare_template_instantiation(T)                                      \
    template Attribute<T> IO::DefineAttribute(                                 \
        const std::string &, const T *, const size_t, const std::string &,     \
        const std::string);                                                    \
                                                                               \
    template Attribute<T> IO::DefineAttribute(const std::string &, const T &,  \
                                              const std::string &,             \
                                              const std::string);              \
                                                                               \
    template Attribute<T> IO::InquireAttribute<T>(                             \
        const std::string &, const std::string &, const std::string);

ADIOS2_FOREACH_ATTRIBUTE_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

} // end namespace adios2
