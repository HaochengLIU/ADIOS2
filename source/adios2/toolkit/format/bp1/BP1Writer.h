/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * BP1Writer.h
 *
 *  Created on: Jan 24, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_TOOLKIT_FORMAT_BP1_BP1WRITER_H_
#define ADIOS2_TOOLKIT_FORMAT_BP1_BP1WRITER_H_

/// \cond EXCLUDE_FROM_DOXYGEN
#include <algorithm> //std::count, std::copy, std::for_each
#include <cmath>     //std::ceil
#include <cstring>   //std::memcpy
/// \endcond

#include "adios2/ADIOSConfig.h"
#include "adios2/ADIOSMacros.h"
#include "adios2/ADIOSTypes.h"
#include "adios2/core/Attribute.h"
#include "adios2/core/IO.h"
#include "adios2/core/Variable.h"
#include "adios2/toolkit/capsule/heap/STLVector.h"
#include "adios2/toolkit/format/bp1/BP1Base.h"
#include "adios2/toolkit/format/bp1/BP1Structs.h"

namespace adios2
{
namespace format
{

class BP1Writer : public BP1Base
{

public:
    /**
     * Unique constructor
     * @param mpiComm MPI communicator for BP1 Aggregator
     * @param debug true: extra checks
     */
    BP1Writer(MPI_Comm mpiComm, const bool debugMode = false);

    ~BP1Writer() = default;

    /**
     * Writes a process group index PGIndex and list of methods (from
     * transports). Done at Open or Advance.
     * @param hostLanguage from ADIOS class passed to IO
     * @param transportsTypes passed to get list of transport "bp methods"
     */
    void WriteProcessGroupIndex(
        const std::string hostLanguage,
        const std::vector<std::string> &transportsTypes) noexcept;

    /**
     * Write metadata for a given variable
     * @param variable
     */
    template <class T>
    void WriteVariableMetadata(const Variable<T> &variable) noexcept;

    /**
     * Expensive part this is only for heap buffers need to adapt to vector of
     * capsules
     * @param variable
     */
    template <class T>
    void WriteVariablePayload(const Variable<T> &variable) noexcept;

    /** Flattens data buffer and closes current process group */
    void Advance(IO &io);

    /** Flattens data buffer and close current process group, doesn't
     *  advance time index */
    void Flush(IO &io);

    /**
     * @param isFirstClose true: first time close, false: already closed buffer
     */
    void Close(IO &io) noexcept;

    /**
     * Get a string with profiling information for this rank
     * @param name stream name
     * @param transportsTypes list of transport types
     * @param transportsProfilers list of references to transport profilers
     */
    std::string GetRankProfilingJSON(
        const std::vector<std::string> &transportsTypes,
        const std::vector<profiling::IOChrono *> &transportsProfilers) noexcept;

    /**
     * Forms the final profiling.json string aggregating from all ranks
     * @param rankProfilingJSON
     * @return profiling.json
     */
    std::string
    AggregateProfilingJSON(const std::string &rankProfilingJSON) noexcept;

private:
    /** BP format version */
    const uint8_t m_Version = 3;

    /**
     * Writes in BP buffer all attributes defined in an IO object.
     * Called by FlattenData function
     * @param io input containing attributes
     */
    void WriteAttributes(IO &io);

    /**
     * Called from WriteAttributeInData specialized functions
     * @param attribute input
     * @param stats
     * @return attribute length position
     */
    template <class T>
    size_t WriteAttributeHeaderInData(const Attribute<T> &attribute,
                                      Stats<T> &stats) noexcept;

    /**
     * Called from WriteAttributeInData specialized functions
     * @param attribute input
     * @param stats
     * @param attributeLengthPosition
     */
    template <class T>
    void
    WriteAttributeLengthInData(const Attribute<T> &attribute, Stats<T> &stats,
                               const size_t attributeLengthPosition) noexcept;

    /**
     * Write a single attribute in data buffer, called from WriteAttributes
     * @param attribute input
     * @param stats
     */
    template <class T>
    void WriteAttributeInData(const Attribute<T> &attribute,
                              Stats<T> &stats) noexcept;

    /**
     * Writes attribute value in index characteristic value.
     * @param characteristicID
     * @param characteristicsCounter
     * @param attribute
     * @param buffer
     */
    template <class T>
    void WriteAttributeCharacteristicValueInIndex(
        std::uint8_t &characteristicsCounter, const Attribute<T> &attribute,
        std::vector<char> &buffer) noexcept;

    /**
     * Write a single attribute in m_Metadata AttributesIndex, called from
     * WriteAttributes
     * @param attribute
     * @param stats
     */
    template <class T>
    void WriteAttributeInIndex(const Attribute<T> &attribute,
                               const Stats<T> &stats) noexcept;

    /**
     * Get variable statistics
     * @param variable
     * @return stats
     */
    template <class T>
    Stats<typename TypeInfo<T>::ValueType>
    GetStats(const Variable<T> &variable) const noexcept;

    template <class T>
    void WriteVariableMetadataInData(
        const Variable<T> &variable,
        const Stats<typename TypeInfo<T>::ValueType> &stats) noexcept;

    template <class T>
    void WriteVariableMetadataInIndex(
        const Variable<T> &variable,
        const Stats<typename TypeInfo<T>::ValueType> &stats, const bool isNew,
        BP1Index &index) noexcept;

    template <class T>
    void WriteVariableCharacteristics(
        const Variable<T> &variable,
        const Stats<typename TypeInfo<T>::ValueType> &stats,
        std::vector<char> &buffer) noexcept;

    template <class T>
    void WriteVariableCharacteristics(
        const Variable<T> &variable,
        const Stats<typename TypeInfo<T>::ValueType> &stats,
        std::vector<char> &buffer, size_t &position) noexcept;

    /**
     * Writes from &buffer[position]:  [2
     * bytes:string.length()][string.length():
     * string.c_str()]
     * @param name to be written in bp file
     * @param buffer metadata buffer
     */
    void WriteNameRecord(const std::string name,
                         std::vector<char> &buffer) noexcept;

    /** Overloaded version for data buffer */
    void WriteNameRecord(const std::string name, std::vector<char> &buffer,
                         size_t &position) noexcept;

    /**
     * Write a dimension record for a global variable used by
     * WriteVariableCommon
     * @param buffer
     * @param position
     * @param localDimensions
     * @param globalDimensions
     * @param offsets
     * @param addType true: for data buffers, false: for metadata buffer and
     * data
     * characteristic
     */
    void WriteDimensionsRecord(const Dims &localDimensions,
                               const Dims &globalDimensions,
                               const Dims &offsets,
                               std::vector<char> &buffer) noexcept;

    /** Overloaded version for data buffer */
    void WriteDimensionsRecord(const Dims &localDimensions,
                               const Dims &globalDimensions,
                               const Dims &offsets, std::vector<char> &buffer,
                               size_t &position,
                               const bool isCharacteristic = false) noexcept;

    /** Writes min max */
    template <class T>
    void WriteBoundsRecord(const bool isScalar, const Stats<T> &stats,
                           uint8_t &characteristicsCounter,
                           std::vector<char> &buffer) noexcept;

    /** Overloaded version for data buffer */
    template <class T>
    void WriteBoundsRecord(const bool singleValue, const Stats<T> &stats,
                           uint8_t &characteristicsCounter,
                           std::vector<char> &buffer,
                           size_t &position) noexcept;

    /**
     * Write a characteristic value record to buffer
     * @param id
     * @param value
     * @param buffers
     * @param positions
     * @param characvteristicsCounter to be updated by 1
     * @param addLength true for data, false for metadata
     */
    template <class T>
    void WriteCharacteristicRecord(const uint8_t characteristicID,
                                   uint8_t &characteristicsCounter,
                                   const T &value,
                                   std::vector<char> &buffer) noexcept;

    /** Overloaded version for data buffer */
    template <class T>
    void WriteCharacteristicRecord(const uint8_t characteristicID,
                                   uint8_t &characteristicsCounter,
                                   const T &value, std::vector<char> &buffer,
                                   size_t &position) noexcept;

    /**
     * Returns corresponding index of type BP1Index, if doesn't exists creates a
     * new one. Used for variables and attributes
     * @param name variable or attribute name to look for index
     * @param indices look up hash table of indices
     * @param isNew true: index is newly created, false: index already exists in
     * indices
     * @return reference to BP1Index in indices
     */
    BP1Index &GetBP1Index(const std::string name,
                          std::unordered_map<std::string, BP1Index> &indices,
                          bool &isNew) const noexcept;

    /**
     * Flattens the data and fills the pg length, vars count, vars length and
     * attributes
     * @param metadataSet
     * @param buffer
     */
    void FlattenData(IO &io) noexcept;

    /**
     * Flattens the metadata indices into a single metadata buffer in capsule
     * @param metadataSet
     * @param buffer
     */
    void FlattenMetadata() noexcept;
};

#define declare_template_instantiation(T)                                      \
    extern template void BP1Writer::WriteVariablePayload(                      \
        const Variable<T> &variable) noexcept;                                 \
                                                                               \
    extern template void BP1Writer::WriteVariableMetadata(                     \
        const Variable<T> &variable) noexcept;

ADIOS2_FOREACH_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

} // end namespace format
} // end namespace adios2

#endif /* ADIOS2_UTILITIES_FORMAT_BP1_BP1WRITER_H_ */
