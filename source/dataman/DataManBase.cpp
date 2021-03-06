/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * DataManBase.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Jason Wang
 */

#include "DataManBase.h"

#include <sstream>

#include <adios2sys/DynamicLoader.hxx>

struct DataManBase::ManagerLibrary
{
    std::string m_LibraryName;
    adios2sys::DynamicLoader::LibraryHandle m_LibraryHandle;
    DataManBase *(*m_getManFunc)();

    ManagerLibrary(std::string method)
    {
        std::vector<std::string> searchedLibs;
        std::string libName;

        std::vector<std::string> libPrefixes;
        libPrefixes.emplace_back("");
        libPrefixes.emplace_back("lib");
#ifdef __CYGWIN__
        libPrefixes.emplace_back("cyg");
#endif

        std::vector<std::string> libSuffixes;
#ifdef __APPLE__
        libSuffixes.emplace_back("man.dylib");
        libSuffixes.emplace_back("man.so");
#endif
#ifdef __hpux
        libSuffixes.emplace_back("man.sl");
#endif
#ifdef __unix__
        libSuffixes.emplace_back("man.so");
#endif
#ifdef _WIN32
        libSuffixes.emplace_back("man.dll");
#endif

        // Test the various combinations of library names
        for (const std::string &prefix : libPrefixes)
        {
            for (const std::string &suffix : libSuffixes)
            {
                libName = prefix + method + suffix;
                m_LibraryHandle =
                    adios2sys::DynamicLoader::OpenLibrary(libName);
                searchedLibs.push_back(libName);
                if (m_LibraryHandle)
                {
                    break;
                }
            }
            if (m_LibraryHandle)
            {
                break;
            }
        }
        if (!m_LibraryHandle)
        {
            std::stringstream errString;
            errString << "Unable to locate the " << method << " manager "
                      << "library; searched for ";
            std::copy(searchedLibs.begin(), searchedLibs.end(),
                      std::ostream_iterator<std::string>(errString, " "));

            throw std::runtime_error(errString.str());
        }

        // Bind to the getMan symbol
        adios2sys::DynamicLoader::SymbolPointer symbolHandle =
            adios2sys::DynamicLoader::GetSymbolAddress(m_LibraryHandle,
                                                       "getMan");
        if (!symbolHandle)
        {
            throw std::runtime_error("Unable to locate the getMan symbol in " +
                                     libName);
        }
        m_getManFunc = reinterpret_cast<DataManBase *(*)()>(symbolHandle);
        m_LibraryName = libName;
    }

    ~ManagerLibrary()
    {
        if (m_LibraryHandle)
        {
            adios2sys::DynamicLoader::CloseLibrary(m_LibraryHandle);
        }
    }
};

DataManBase::DataManBase()
{
    m_profiling["total_manager_time"] = 0.0f;
    m_profiling["total_mb"] = 0.0f;
    m_start_time = std::chrono::system_clock::now();
}

int DataManBase::put_begin(const void *p_data, json &p_jmsg)
{
    check_shape(p_jmsg);
    if (p_jmsg["compressed_size"].is_number())
    {
        p_jmsg["sendbytes"] = p_jmsg["compressed_size"].get<size_t>();
    }
    else
    {
        p_jmsg["sendbytes"] = p_jmsg["putbytes"].get<size_t>();
    }
    p_jmsg["profiling"] = m_profiling;
    m_step_time = std::chrono::system_clock::now();
    return 0;
}

int DataManBase::put_end(const void *p_data, json &p_jmsg)
{
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end - m_step_time;
    m_profiling["total_manager_time"] =
        m_profiling["total_manager_time"].get<double>() + duration.count();
    m_profiling["total_mb"] =
        m_profiling["total_mb"].get<double>() +
        product(p_jmsg["varshape"], dsize(p_jmsg["dtype"])) / 1000000.0f;
    duration = end - m_start_time;
    m_profiling["total_workflow_time"] = duration.count();
    m_profiling["workflow_mbs"] =
        m_profiling["total_mb"].get<double>() /
        m_profiling["total_workflow_time"].get<double>();
    m_profiling["manager_mbs"] =
        m_profiling["total_mb"].get<double>() /
        m_profiling["total_manager_time"].get<double>();
    return 0;
}

void DataManBase::reg_callback(
    std::function<void(const void *, std::string, std::string, std::string,
                       std::vector<size_t>)>
        cb)
{
    if (m_stream_mans.empty())
    {
        m_callback = cb;
    }
    else
    {
        for (const auto &i : m_stream_mans)
        {
            i->reg_callback(cb);
        }
    }
}

void DataManBase::dump(const void *p_data, json p_jmsg, std::ostream &out)
{
    std::vector<size_t> p_varshape =
        p_jmsg["varshape"].get<std::vector<size_t>>();
    std::string dtype = p_jmsg["dtype"];
    size_t length = p_jmsg["dumplength"].get<size_t>();
    size_t s = 0;
    for (size_t i = 0; i < product(p_varshape, 1); ++i)
    {
        s++;
        out << (static_cast<const float *>(p_data))[i] << " ";
        if (s == length)
        {
            out << std::endl;
            s = 0;
        }
    }
    out << std::endl;
}

bool DataManBase::auto_transform(std::vector<char> &a_data, json &a_jmsg)
{
    if (a_jmsg["compression_method"].is_string() &&
        a_jmsg["compression_method"].get<std::string>() != "null")
    {
        auto method = a_jmsg["compression_method"].get<std::string>();
        auto man = get_man(method);
        if (!man)
        {
            logging("Library file for compression method " + method +
                    " not found!");
            return false;
        }
        man->transform(a_data, a_jmsg);
        a_jmsg.erase("compression_method");
        a_jmsg.erase("compression_rate");
        a_jmsg.erase("compressed_size");
        return true;
    }
    else
    {
        return false;
    }
}

std::shared_ptr<DataManBase> DataManBase::get_man(std::string method)
{
    try
    {
        // Reuse already loaded libraries if possible
        auto libIt = m_LoadedManagers.find(method);
        if (libIt == m_LoadedManagers.end())
        {
            // This insertion will only fail if an entry for method already
            // exists, which this if block ensures that it doesn't.
            libIt =
                m_LoadedManagers.insert({method, new ManagerLibrary(method)})
                    .first;
            logging("Loaded " + libIt->second->m_LibraryName);
        }
        else
        {
            logging("Using existing " + libIt->second->m_LibraryName + ".");
        }
        return std::shared_ptr<DataManBase>(libIt->second->m_getManFunc());
    }
    catch (const std::runtime_error &ex)
    {
        logging(ex.what());
        return nullptr;
    }
}

void DataManBase::logging(std::string p_msg, std::string p_man,
                          std::ostream &out)
{
    if (p_man == "")
    {
        p_man = name();
    }
    out << "[";
    out << p_man;
    out << "]";
    out << " ";
    out << p_msg;
    out << std::endl;
}

bool DataManBase::check_json(json p_jmsg, std::vector<std::string> p_strings,
                             std::string p_man)
{
    if (p_man == "")
    {
        p_man = name();
    }
    for (const auto &i : p_strings)
    {
        if (p_jmsg[i] == nullptr)
        {
            if (p_man != "")
            {
                logging("JSON key " + i + " not found!", p_man);
            }
            return false;
        }
    }
    return true;
}

size_t DataManBase::product(size_t *shape)
{
    size_t s = 1;
    if (shape)
    {
        for (size_t i = 1; i <= shape[0]; ++i)
        {
            s *= shape[i];
        }
    }
    return s;
}

size_t DataManBase::product(std::vector<size_t> shape, size_t size)
{
    return accumulate(shape.begin(), shape.end(), size,
                      std::multiplies<size_t>());
}

size_t DataManBase::dsize(std::string dtype)
{
    if (dtype == "char")
    {
        return sizeof(char);
    }
    if (dtype == "short")
    {
        return sizeof(short);
    }
    if (dtype == "int")
    {
        return sizeof(int);
    }
    if (dtype == "long")
    {
        return sizeof(long);
    }
    if (dtype == "unsigned char")
    {
        return sizeof(unsigned char);
    }
    if (dtype == "unsigned short")
    {
        return sizeof(unsigned short);
    }
    if (dtype == "unsigned int")
    {
        return sizeof(unsigned int);
    }
    if (dtype == "unsigned long")
    {
        return sizeof(unsigned long);
    }
    if (dtype == "float")
    {
        return sizeof(float);
    }
    if (dtype == "double")
    {
        return sizeof(double);
    }
    if (dtype == "long double")
    {
        return sizeof(long double);
    }
    if (dtype == "std::complex<float>" or dtype == "complex<float>")
    {
        return sizeof(std::complex<float>);
    }
    if (dtype == "std::complex<double>")
    {
        return sizeof(std::complex<double>);
    }

    if (dtype == "int8_t")
    {
        return sizeof(int8_t);
    }
    if (dtype == "uint8_t")
    {
        return sizeof(uint8_t);
    }
    if (dtype == "int16_t")
    {
        return sizeof(int16_t);
    }
    if (dtype == "uint16_t")
    {
        return sizeof(uint16_t);
    }
    if (dtype == "int32_t")
    {
        return sizeof(int32_t);
    }
    if (dtype == "uint32_t")
    {
        return sizeof(uint32_t);
    }
    if (dtype == "int64_t")
    {
        return sizeof(int64_t);
    }
    if (dtype == "uint64_t")
    {
        return sizeof(uint64_t);
    }
    return 0;
}

int DataManBase::closest(int v, json j, bool up)
{
    int s = 100, k = 0, t;
    for (unsigned int i = 0; i < j.size(); ++i)
    {
        if (up)
        {
            t = j[i].get<int>() - v;
        }
        else
        {
            t = v - j[i].get<int>();
        }
        if (t >= 0 && t < s)
        {
            s = t;
            k = i;
        }
    }
    return k;
}

void DataManBase::check_shape(json &p_jmsg)
{
    std::vector<size_t> varshape;
    if (check_json(p_jmsg, {"varshape"}))
    {
        varshape = p_jmsg["varshape"].get<std::vector<size_t>>();
    }
    else
    {
        return;
    }
    if (not p_jmsg["putshape"].is_array())
    {
        p_jmsg["putshape"] = varshape;
    }
    if (not p_jmsg["offset"].is_array())
    {
        p_jmsg["offset"] = std::vector<size_t>(varshape.size(), 0);
    }
    p_jmsg["dsize"] = dsize(p_jmsg["dtype"].get<std::string>());

    p_jmsg["putsize"] = product(p_jmsg["putshape"].get<std::vector<size_t>>());
    p_jmsg["varsize"] = product(varshape);

    p_jmsg["putbytes"] = product(p_jmsg["putshape"].get<std::vector<size_t>>(),
                                 dsize(p_jmsg["dtype"].get<std::string>()));
    p_jmsg["varbytes"] =
        product(varshape, dsize(p_jmsg["dtype"].get<std::string>()));
}

void DataManBase::dump_profiling() { logging(m_profiling.dump(4)); }
