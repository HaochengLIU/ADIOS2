/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#include <cstdint>
#include <cstring>

#include <iostream>
#include <stdexcept>

#include <adios.h>
#include <adios2.h>
#include <adios_read.h>

#include <gtest/gtest.h>

#include "../SmallTestData.h"

#ifdef ADIOS2_HAVE_MPI
    #include "mpi.h"
#endif

class ADIOS1WriteReadTest : public ::testing::Test
{
public:
    ADIOS1WriteReadTest() = default;

    const SmallTestData m_TestData;
};

//******************************************************************************
// 1D 1x8 test data
//******************************************************************************

// ADIOS2 write using ADIOS1Writer, native ADIOS1 read
TEST_F(ADIOS1WriteReadTest, ADIOS2ADIOS1WriteADIOS1Read1D8)
{
    // Each process would write a 1x8 array and all processes would
    // form a world_size * Nx matrix
    std::string fname = "ADIOS2ADIOS1WriteADIOS1Read1D8.bp";

    int world_rank = 0, world_size = 1;
    // Number of rows
    const std::size_t Nx = 8;

    // Number of steps
    const std::size_t NSteps = 3;

#ifdef ADIOS2_HAVE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
#endif

    // Write test data using ADIOS2
    {
#ifdef ADIOS2_HAVE_MPI
        adios_init_noxml(MPI_COMM_WORLD);

        adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
#else
        adios2::ADIOS adios(true);
#endif
        adios2::IO &io = adios.DeclareIO("TestIO");

        // Declare 2D variables (NumOfProcesses * Nx)
        // The local process' part (start, count) can be defined now or later
        // before Write().
        {
            adios2::Dims shape = adios2::Dims{(unsigned int) world_size, Nx};
            auto &var_i8 =
                io.DefineVariable<char>("i8", shape);
            auto &var_i16 =
                io.DefineVariable<short>("i16",shape);
            auto &var_i32 =
                io.DefineVariable<int>("i32", shape);
            auto &var_i64 =
                io.DefineVariable<long>("i64", shape);
            auto &var_u8 =
                io.DefineVariable<unsigned char>("u8", shape);
            auto &var_u16 = io.DefineVariable<unsigned short>("u16", shape);
            auto &var_u32 =
                io.DefineVariable<unsigned int>("u32", shape);
            auto &var_u64 = io.DefineVariable<unsigned long>("u64", shape);
            auto &var_r32 =
                io.DefineVariable<float>("r32", shape);
            auto &var_r64 =
                io.DefineVariable<double>("r64", shape);
        }

        // Create the ADIOS 1 Engine
        io.SetEngine("ADIOS1Writer");
#ifdef ADIOS2_HAVE_MPI
        io.AddTransport("file", {{"library", "MPI"}});
#else
        io.AddTransport("file");
#endif

        auto engine = io.Open(fname, adios2::OpenMode::Write);
        ASSERT_NE(engine.get(), nullptr);

        for (size_t step = 0; step < NSteps; ++step)
        {
            // Generate test data for each process uniquely
            SmallTestData currentTestData = generateNewSmallTestData(m_TestData,
                                                                     step,
                                                                     world_rank,
                                                                    world_size);

            // Retrieve the variables that previously went out of scope
            auto &var_i8 = io.GetVariable<char>("i8");
            auto &var_i16 = io.GetVariable<short>("i16");
            auto &var_i32 = io.GetVariable<int>("i32");
            auto &var_i64 = io.GetVariable<long>("i64");
            auto &var_u8 = io.GetVariable<unsigned char>("u8");
            auto &var_u16 = io.GetVariable<unsigned short>("u16");
            auto &var_u32 = io.GetVariable<unsigned int>("u32");
            auto &var_u64 = io.GetVariable<unsigned long>("u64");
            auto &var_r32 = io.GetVariable<float>("r32");
            auto &var_r64 = io.GetVariable<double>("r64");

            // Make a 2D selection to describe the local dimensions of the
            // variable we write and its offsets in the global spaces
            adios2::SelectionBoundingBox sel({(unsigned int)world_rank, 0}, {1, Nx});
            var_i8.SetSelection(sel);
            var_i16.SetSelection(sel);
            var_i32.SetSelection(sel);
            var_i64.SetSelection(sel);
            var_u8.SetSelection(sel);
            var_u16.SetSelection(sel);
            var_u32.SetSelection(sel);
            var_u64.SetSelection(sel);
            var_r32.SetSelection(sel);
            var_r64.SetSelection(sel);

            // Write each one
            // fill in the variable with values from starting index to
            // starting index + count
            engine->Write(var_i8, currentTestData.I8.data());
            engine->Write(var_i16, currentTestData.I16.data());
            engine->Write(var_i32, currentTestData.I32.data());
            engine->Write(var_i64, currentTestData.I64.data());
            engine->Write(var_u8, currentTestData.U8.data());
            engine->Write(var_u16, currentTestData.U16.data());
            engine->Write(var_u32, currentTestData.U32.data());
            engine->Write(var_u64, currentTestData.U64.data());
            engine->Write(var_r32, currentTestData.R32.data());
            engine->Write(var_r64, currentTestData.R64.data());

            // Advance to the next time step
            // Question
            engine->Advance();
        }

        // Close the file
        engine->Close();

        adios_finalize(0);
    } // indent here

    {
        adios_read_init_method(ADIOS_READ_METHOD_BP, MPI_COMM_WORLD,
                               "verbose=3");

        // Open the file for reading
        ADIOS_FILE *f = adios_read_open_file(
            fname.c_str(), ADIOS_READ_METHOD_BP, MPI_COMM_WORLD);
        ASSERT_NE(f, nullptr);

        // Check the variables exist
        ADIOS_VARINFO *var_i8 = adios_inq_var(f, "i8");
        ASSERT_NE(var_i8, nullptr);
        ASSERT_EQ(var_i8->ndim, 2);
        ASSERT_EQ(var_i8->dims[0], world_size);
        ASSERT_EQ(var_i8->dims[1], Nx);
        ADIOS_VARINFO *var_i16 = adios_inq_var(f, "i16");
        ASSERT_NE(var_i16, nullptr);
        ASSERT_EQ(var_i16->ndim, 2);
        ASSERT_EQ(var_i16->dims[0], world_size);
        ASSERT_EQ(var_i16->dims[1], Nx);
        ADIOS_VARINFO *var_i32 = adios_inq_var(f, "i32");
        ASSERT_NE(var_i32, nullptr);
        ASSERT_EQ(var_i32->ndim, 2);
        ASSERT_EQ(var_i32->dims[0], world_size);
        ASSERT_EQ(var_i32->dims[1], Nx);
        ADIOS_VARINFO *var_i64 = adios_inq_var(f, "i64");
        ASSERT_NE(var_i64, nullptr);
        ASSERT_EQ(var_i64->ndim, 2);
        ASSERT_EQ(var_i64->dims[0], world_size);
        ASSERT_EQ(var_i64->dims[1], Nx);
        ADIOS_VARINFO *var_u8 = adios_inq_var(f, "u8");
        ASSERT_NE(var_u8, nullptr);
        ASSERT_EQ(var_u8->ndim, 2);
        ASSERT_EQ(var_u8->dims[0], world_size);
        ASSERT_EQ(var_u8->dims[1], Nx);
        ADIOS_VARINFO *var_u16 = adios_inq_var(f, "u16");
        ASSERT_NE(var_u16, nullptr);
        ASSERT_EQ(var_u16->ndim, 2);
        ASSERT_EQ(var_u16->dims[0], world_size);
        ASSERT_EQ(var_u16->dims[1], Nx);
        ADIOS_VARINFO *var_u32 = adios_inq_var(f, "u32");
        ASSERT_NE(var_u32, nullptr);
        ASSERT_EQ(var_u32->ndim, 2);
        ASSERT_EQ(var_u32->dims[0], world_size);
        ASSERT_EQ(var_u32->dims[1], Nx);
        ADIOS_VARINFO *var_u64 = adios_inq_var(f, "u64");
        ASSERT_NE(var_u64, nullptr);
        ASSERT_EQ(var_u64->ndim, 2);
        ASSERT_EQ(var_u64->dims[0], world_size);
        ASSERT_EQ(var_u64->dims[1], Nx);
        ADIOS_VARINFO *var_r32 = adios_inq_var(f, "r32");
        ASSERT_NE(var_r32, nullptr);
        ASSERT_EQ(var_r32->ndim, 2);
        ASSERT_EQ(var_r32->dims[0], world_size);
        ASSERT_EQ(var_r32->dims[1], Nx);
        ADIOS_VARINFO *var_r64 = adios_inq_var(f, "r64");
        ASSERT_NE(var_r64, nullptr);
        ASSERT_EQ(var_r64->ndim, 2);
        ASSERT_EQ(var_r64->dims[0], world_size);
        ASSERT_EQ(var_r64->dims[1], Nx);

        std::array<char, Nx> I8;
        std::array<int16_t, Nx> I16;
        std::array<int32_t, Nx> I32;
        std::array<int64_t, Nx> I64;
        std::array<unsigned char, Nx> U8;
        std::array<uint16_t, Nx> U16;
        std::array<uint32_t, Nx> U32;
        std::array<uint64_t, Nx> U64;
        std::array<float, Nx> R32;
        std::array<double, Nx> R64;

        uint64_t start[2] = {static_cast<uint64_t>(world_rank), 0};
        uint64_t count[2] = {1, Nx};
        ADIOS_SELECTION *sel = adios_selection_boundingbox(2, start, count);

        // Read stuff
        for (size_t t = 0; t < NSteps; ++t)
        {
            // Generate test data for each rank uniquely
            SmallTestData currentTestData = generateNewSmallTestData(m_TestData,
                                                                     t,
                                                                     world_rank,
                                                                    world_size);
            // Read the current step
            adios_schedule_read_byid(f, sel, var_i8->varid, t, 1, I8.data());
            adios_schedule_read_byid(f, sel, var_i16->varid, t, 1, I16.data());
            adios_schedule_read_byid(f, sel, var_i32->varid, t, 1, I32.data());
            adios_schedule_read_byid(f, sel, var_i64->varid, t, 1, I64.data());
            adios_schedule_read_byid(f, sel, var_u8->varid, t, 1, U8.data());
            adios_schedule_read_byid(f, sel, var_u16->varid, t, 1, U16.data());
            adios_schedule_read_byid(f, sel, var_u32->varid, t, 1, U32.data());
            adios_schedule_read_byid(f, sel, var_u64->varid, t, 1, U64.data());
            adios_schedule_read_byid(f, sel, var_r32->varid, t, 1, R32.data());
            adios_schedule_read_byid(f, sel, var_r64->varid, t, 1, R64.data());
            adios_perform_reads(f, 1);

            // Check if it's correct
            for (size_t i = 0; i < Nx; ++i)
            {
                std::stringstream ss;
                ss << "t=" << t << " i=" << i << " rank=" << world_rank;
                std::string msg = ss.str();

                EXPECT_EQ(I8[i], currentTestData.I8[i]) << msg;
                EXPECT_EQ(I16[i], currentTestData.I16[i]) << msg;
                EXPECT_EQ(I32[i], currentTestData.I32[i]) << msg;
                EXPECT_EQ(I64[i], currentTestData.I64[i]) << msg;
                EXPECT_EQ(U8[i], currentTestData.U8[i]) << msg;
                EXPECT_EQ(U16[i], currentTestData.U16[i]) << msg;
                EXPECT_EQ(U32[i], currentTestData.U32[i]) << msg;
                EXPECT_EQ(U64[i], currentTestData.U64[i]) << msg;
                EXPECT_EQ(R32[i], currentTestData.R32[i]) << msg;
                EXPECT_EQ(R64[i], currentTestData.R64[i]) << msg;
            }
        }

        adios_selection_delete(sel);

        // Cleanup variable structures
        adios_free_varinfo(var_i8);
        adios_free_varinfo(var_i16);
        adios_free_varinfo(var_i32);
        adios_free_varinfo(var_i64);
        adios_free_varinfo(var_u8);
        adios_free_varinfo(var_u16);
        adios_free_varinfo(var_u32);
        adios_free_varinfo(var_u64);
        adios_free_varinfo(var_r32);
        adios_free_varinfo(var_r64);

        // Cleanup file
        adios_read_close(f);

        adios_read_finalize_method(ADIOS_READ_METHOD_BP);
    }
}

// ADIOS2 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS2ADIOS1WriteADIOS2ADIOS1Read1D8)
{
    std::string fname = "ADIOS2ADIOS1WriteADIOS2ADIOS1Read1D8.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

// Native ADIOS1 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS1WriteADIOS2ADIOS1Read1D8)
{
    std::string fname = "ADIOS1WriteADIOS2ADIOS1Read1D8.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

//******************************************************************************
// 2D 2x4 test data
//******************************************************************************

// ADIOS2 write, native ADIOS1 read
TEST_F(ADIOS1WriteReadTest, ADIOS2ADIOS1WriteADIOS1Read2D2x4)
{
    std::string fname = "ADIOS2ADIOS1WriteADIOS1Read2D2x4Test.bp";

    // Write test data using ADIOS2
    {
        adios_init_noxml(MPI_COMM_WORLD);

        adios2::ADIOS adios(true);
        adios2::IO &io = adios.DeclareIO("TestIO");

        // Declare 1D variables
        {
            auto &var_i8 =
                io.DefineVariable<char>("i8", {}, {}, adios2::Dims{2, 4});
            auto &var_i16 =
                io.DefineVariable<short>("i16", {}, {}, adios2::Dims{2, 4});
            auto &var_i32 =
                io.DefineVariable<int>("i32", {}, {}, adios2::Dims{2, 4});
            auto &var_i64 =
                io.DefineVariable<long>("i64", {}, {}, adios2::Dims{2, 4});
            auto &var_u8 = io.DefineVariable<unsigned char>("u8", {}, {},
                                                            adios2::Dims{2, 4});
            auto &var_u16 = io.DefineVariable<unsigned short>(
                "u16", {}, {}, adios2::Dims{2, 4});
            auto &var_u32 = io.DefineVariable<unsigned int>("u32", {}, {},
                                                            adios2::Dims{2, 4});
            auto &var_u64 = io.DefineVariable<unsigned long>(
                "u64", {}, {}, adios2::Dims{2, 4});
            auto &var_r32 =
                io.DefineVariable<float>("r32", {}, {}, adios2::Dims{2, 4});
            auto &var_r64 =
                io.DefineVariable<double>("r64", {}, {}, adios2::Dims{2, 4});
        }

        // Create the ADIOS 1 Engine
        io.SetEngine("ADIOS1Writer");
        io.AddTransport("file");

        auto engine = io.Open(fname, adios2::OpenMode::Write);
        ASSERT_NE(engine.get(), nullptr);

        for (size_t step = 0; step < 3; ++step)
        {
            // Retrieve the variables that previously went out of scope
            auto &var_i8 = io.GetVariable<char>("i8");
            auto &var_i16 = io.GetVariable<short>("i16");
            auto &var_i32 = io.GetVariable<int>("i32");
            auto &var_i64 = io.GetVariable<long>("i64");
            auto &var_u8 = io.GetVariable<unsigned char>("u8");
            auto &var_u16 = io.GetVariable<unsigned short>("u16");
            auto &var_u32 = io.GetVariable<unsigned int>("u32");
            auto &var_u64 = io.GetVariable<unsigned long>("u64");
            auto &var_r32 = io.GetVariable<float>("r32");
            auto &var_r64 = io.GetVariable<double>("r64");

            // Write each one
            engine->Write(var_i8, m_TestData.I8.data() + step);
            engine->Write(var_i16, m_TestData.I16.data() + step);
            engine->Write(var_i32, m_TestData.I32.data() + step);
            engine->Write(var_i64, m_TestData.I64.data() + step);
            engine->Write(var_u8, m_TestData.U8.data() + step);
            engine->Write(var_u16, m_TestData.U16.data() + step);
            engine->Write(var_u32, m_TestData.U32.data() + step);
            engine->Write(var_u64, m_TestData.U64.data() + step);
            engine->Write(var_r32, m_TestData.R32.data() + step);
            engine->Write(var_r64, m_TestData.R64.data() + step);

            // Advance to the next time step
            engine->Advance();
        }

        // Close the file
        engine->Close();

        adios_finalize(0);
    }

// Read test data using ADIOS1
//#ifdef ADIOS2_HAVE_MPI
//    // Read everything from rank 0
//    int rank;
//    MPI_Comm_rank();
//    if (rank == 0)
//#endif
    {
        adios_read_init_method(ADIOS_READ_METHOD_BP, MPI_COMM_WORLD,
                               "verbose=3");

        // Open the file for reading
        ADIOS_FILE *f = adios_read_open_file(
            fname.c_str(), ADIOS_READ_METHOD_BP, MPI_COMM_WORLD);
        ASSERT_NE(f, nullptr);

        // Check the variables exist
        ADIOS_VARINFO *var_i8 = adios_inq_var(f, "i8");
        ASSERT_NE(var_i8, nullptr);
        ASSERT_EQ(var_i8->ndim, 2);
        ASSERT_EQ(var_i8->dims[0], 2);
        ASSERT_EQ(var_i8->dims[1], 4);
        ADIOS_VARINFO *var_i16 = adios_inq_var(f, "i16");
        ASSERT_NE(var_i16, nullptr);
        ASSERT_EQ(var_i16->ndim, 2);
        ASSERT_EQ(var_i16->dims[0], 2);
        ASSERT_EQ(var_i16->dims[1], 4);
        ADIOS_VARINFO *var_i32 = adios_inq_var(f, "i32");
        ASSERT_NE(var_i32, nullptr);
        ASSERT_EQ(var_i32->ndim, 2);
        ASSERT_EQ(var_i32->dims[0], 2);
        ASSERT_EQ(var_i32->dims[1], 4);
        ADIOS_VARINFO *var_i64 = adios_inq_var(f, "i64");
        ASSERT_NE(var_i64, nullptr);
        ASSERT_EQ(var_i64->ndim, 2);
        ASSERT_EQ(var_i64->dims[0], 2);
        ASSERT_EQ(var_i64->dims[1], 4);
        ADIOS_VARINFO *var_u8 = adios_inq_var(f, "u8");
        ASSERT_NE(var_u8, nullptr);
        ASSERT_EQ(var_u8->ndim, 2);
        ASSERT_EQ(var_u8->dims[0], 2);
        ASSERT_EQ(var_u8->dims[1], 4);
        ADIOS_VARINFO *var_u16 = adios_inq_var(f, "u16");
        ASSERT_NE(var_u16, nullptr);
        ASSERT_EQ(var_u16->ndim, 2);
        ASSERT_EQ(var_u16->dims[0], 2);
        ASSERT_EQ(var_u16->dims[1], 4);
        ADIOS_VARINFO *var_u32 = adios_inq_var(f, "u32");
        ASSERT_NE(var_u32, nullptr);
        ASSERT_EQ(var_u32->ndim, 2);
        ASSERT_EQ(var_u32->dims[0], 2);
        ASSERT_EQ(var_u32->dims[1], 4);
        ADIOS_VARINFO *var_u64 = adios_inq_var(f, "u64");
        ASSERT_NE(var_u64, nullptr);
        ASSERT_EQ(var_u64->ndim, 2);
        ASSERT_EQ(var_u64->dims[0], 2);
        ASSERT_EQ(var_u64->dims[1], 4);
        ADIOS_VARINFO *var_r32 = adios_inq_var(f, "r32");
        ASSERT_NE(var_r32, nullptr);
        ASSERT_EQ(var_r32->ndim, 2);
        ASSERT_EQ(var_r32->dims[0], 2);
        ASSERT_EQ(var_r32->dims[1], 4);
        ADIOS_VARINFO *var_r64 = adios_inq_var(f, "r64");
        ASSERT_NE(var_r64, nullptr);
        ASSERT_EQ(var_r64->ndim, 2);
        ASSERT_EQ(var_r64->dims[0], 2);
        ASSERT_EQ(var_r64->dims[1], 4);

        std::array<char, 8> I8;
        std::array<int16_t, 8> I16;
        std::array<int32_t, 8> I32;
        std::array<int64_t, 8> I64;
        std::array<unsigned char, 8> U8;
        std::array<uint16_t, 8> U16;
        std::array<uint32_t, 8> U32;
        std::array<uint64_t, 8> U64;
        std::array<float, 8> R32;
        std::array<double, 8> R64;

        uint64_t start[2] = {0, 0};
        uint64_t count[2] = {2, 4};
        ADIOS_SELECTION *sel = adios_selection_boundingbox(2, start, count);

        // Read stuff
        for (size_t t = 0; t < 3; ++t)
        {
            // Read the current step
            adios_schedule_read_byid(f, sel, var_i8->varid, t, 1, I8.data());
            adios_schedule_read_byid(f, sel, var_i16->varid, t, 1, I16.data());
            adios_schedule_read_byid(f, sel, var_i32->varid, t, 1, I32.data());
            adios_schedule_read_byid(f, sel, var_i64->varid, t, 1, I64.data());
            adios_schedule_read_byid(f, sel, var_u8->varid, t, 1, U8.data());
            adios_schedule_read_byid(f, sel, var_u16->varid, t, 1, U16.data());
            adios_schedule_read_byid(f, sel, var_u32->varid, t, 1, U32.data());
            adios_schedule_read_byid(f, sel, var_u64->varid, t, 1, U64.data());
            adios_schedule_read_byid(f, sel, var_r32->varid, t, 1, R32.data());
            adios_schedule_read_byid(f, sel, var_r64->varid, t, 1, R64.data());
            adios_perform_reads(f, 1);

            // Check if it's correct
            for (size_t i = 0; i < 8; ++i)
            {
                std::stringstream ss;
                ss << "t=" << t << " i=" << i;
                std::string msg = ss.str();

                EXPECT_EQ(I8[i], m_TestData.I8[i + t]) << msg;
                EXPECT_EQ(I16[i], m_TestData.I16[i + t]) << msg;
                EXPECT_EQ(I32[i], m_TestData.I32[i + t]) << msg;
                EXPECT_EQ(I64[i], m_TestData.I64[i + t]) << msg;
                EXPECT_EQ(U8[i], m_TestData.U8[i + t]) << msg;
                EXPECT_EQ(U16[i], m_TestData.U16[i + t]) << msg;
                EXPECT_EQ(U32[i], m_TestData.U32[i + t]) << msg;
                EXPECT_EQ(U64[i], m_TestData.U64[i + t]) << msg;
                EXPECT_EQ(R32[i], m_TestData.R32[i + t]) << msg;
                EXPECT_EQ(R64[i], m_TestData.R64[i + t]) << msg;
            }
        }

        adios_selection_delete(sel);

        // Cleanup variable structures
        adios_free_varinfo(var_i8);
        adios_free_varinfo(var_i16);
        adios_free_varinfo(var_i32);
        adios_free_varinfo(var_i64);
        adios_free_varinfo(var_u8);
        adios_free_varinfo(var_u16);
        adios_free_varinfo(var_u32);
        adios_free_varinfo(var_u64);
        adios_free_varinfo(var_r32);
        adios_free_varinfo(var_r64);

        // Cleanup file
        adios_read_close(f);

        adios_read_finalize_method(ADIOS_READ_METHOD_BP);
    }
}

// ADIOS2 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS2ADIOS1WriteADIOS2ADIOS1Read2D2x4)
{
    std::string fname = "ADIOS2ADIOS1WriteADIOS2ADIOS1Read2D2x4Test.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

// Native ADIOS1 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS1WriteADIOS2ADIOS1Read2D2x4)
{
    std::string fname = "ADIOS1WriteADIOS2ADIOS1Read2D2x4Test.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

//******************************************************************************
// 2D 4x2 test data
//******************************************************************************

// ADIOS2 write, native ADIOS1 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS2ADIOS1WriteADIOS1Read2D4x2)
{
    std::string fname = "ADIOS2ADIOS1WriteADIOS1Read2D4x2Test.bp";

    // Write test data using ADIOS2
    {
        adios_init_noxml(MPI_COMM_WORLD);

        adios2::ADIOS adios(MPI_COMM_WORLD, adios2::DebugON);
        adios2::IO &io = adios.DeclareIO("TestIO");

        // Declare 1D variables
        {
            auto &var_i8 =
                io.DefineVariable<char>("i8", {}, {}, adios2::Dims{4, 2});
            auto &var_i16 =
                io.DefineVariable<short>("i16", {}, {}, adios2::Dims{4, 2});
            auto &var_i32 =
                io.DefineVariable<int>("i32", {}, {}, adios2::Dims{4, 2});
            auto &var_i64 =
                io.DefineVariable<long>("i64", {}, {}, adios2::Dims{4, 2});
            auto &var_u8 = io.DefineVariable<unsigned char>("u8", {}, {},
                                                            adios2::Dims{4, 2});
            auto &var_u16 = io.DefineVariable<unsigned short>(
                "u16", {}, {}, adios2::Dims{4, 2});
            auto &var_u32 = io.DefineVariable<unsigned int>("u32", {}, {},
                                                            adios2::Dims{4, 2});
            auto &var_u64 = io.DefineVariable<unsigned long>(
                "u64", {}, {}, adios2::Dims{4, 2});
            auto &var_r32 =
                io.DefineVariable<float>("r32", {}, {}, adios2::Dims{4, 2});
            auto &var_r64 =
                io.DefineVariable<double>("r64", {}, {}, adios2::Dims{4, 2});
        }

        // Create the ADIOS 1 Engine
        io.SetEngine("ADIOS1Writer");
        // Question: Eh.. why?
        io.AddTransport("file", {{"library", "MPI"}});

        auto engine = io.Open(fname, adios2::OpenMode::Write);
        ASSERT_NE(engine.get(), nullptr);

        for (size_t step = 0; step < 3; ++step)
        {
            // Retrieve the variables that previously went out of scope
            auto &var_i8 = io.GetVariable<char>("i8");
            auto &var_i16 = io.GetVariable<short>("i16");
            auto &var_i32 = io.GetVariable<int>("i32");
            auto &var_i64 = io.GetVariable<long>("i64");
            auto &var_u8 = io.GetVariable<unsigned char>("u8");
            auto &var_u16 = io.GetVariable<unsigned short>("u16");
            auto &var_u32 = io.GetVariable<unsigned int>("u32");
            auto &var_u64 = io.GetVariable<unsigned long>("u64");
            auto &var_r32 = io.GetVariable<float>("r32");
            auto &var_r64 = io.GetVariable<double>("r64");

            // Write each one
            engine->Write(var_i8, m_TestData.I8.data() + step);
            engine->Write(var_i16, m_TestData.I16.data() + step);
            engine->Write(var_i32, m_TestData.I32.data() + step);
            engine->Write(var_i64, m_TestData.I64.data() + step);
            engine->Write(var_u8, m_TestData.U8.data() + step);
            engine->Write(var_u16, m_TestData.U16.data() + step);
            engine->Write(var_u32, m_TestData.U32.data() + step);
            engine->Write(var_u64, m_TestData.U64.data() + step);
            engine->Write(var_r32, m_TestData.R32.data() + step);
            engine->Write(var_r64, m_TestData.R64.data() + step);

            // Advance to the next time step
            engine->Advance();
        }

        // Close the file
        engine->Close();

        adios_finalize(0);
    }

// Read test data using ADIOS1
//#ifdef ADIOS2_HAVE_MPI
//    // Read everything from rank 0
//    int rank;
//    MPI_Comm_rank();
//    if (rank == 0)
//#endif
    {
        adios_read_init_method(ADIOS_READ_METHOD_BP, MPI_COMM_WORLD,
                               "verbose=3");

        // Open the file for reading
        ADIOS_FILE *f = adios_read_open_file(
            fname.c_str(), ADIOS_READ_METHOD_BP, MPI_COMM_WORLD);
        ASSERT_NE(f, nullptr);

        // Check the variables exist
        ADIOS_VARINFO *var_i8 = adios_inq_var(f, "i8");
        ASSERT_NE(var_i8, nullptr);
        ASSERT_EQ(var_i8->ndim, 2);
        ASSERT_EQ(var_i8->dims[0], 4);
        ASSERT_EQ(var_i8->dims[1], 2);
        ADIOS_VARINFO *var_i16 = adios_inq_var(f, "i16");
        ASSERT_NE(var_i16, nullptr);
        ASSERT_EQ(var_i16->ndim, 2);
        ASSERT_EQ(var_i16->dims[0], 4);
        ASSERT_EQ(var_i16->dims[1], 2);
        ADIOS_VARINFO *var_i32 = adios_inq_var(f, "i32");
        ASSERT_NE(var_i32, nullptr);
        ASSERT_EQ(var_i32->ndim, 2);
        ASSERT_EQ(var_i32->dims[0], 4);
        ASSERT_EQ(var_i32->dims[1], 2);
        ADIOS_VARINFO *var_i64 = adios_inq_var(f, "i64");
        ASSERT_NE(var_i64, nullptr);
        ASSERT_EQ(var_i64->ndim, 2);
        ASSERT_EQ(var_i64->dims[0], 4);
        ASSERT_EQ(var_i64->dims[1], 2);
        ADIOS_VARINFO *var_u8 = adios_inq_var(f, "u8");
        ASSERT_NE(var_u8, nullptr);
        ASSERT_EQ(var_u8->ndim, 2);
        ASSERT_EQ(var_u8->dims[0], 4);
        ASSERT_EQ(var_u8->dims[1], 2);
        ADIOS_VARINFO *var_u16 = adios_inq_var(f, "u16");
        ASSERT_NE(var_u16, nullptr);
        ASSERT_EQ(var_u16->ndim, 2);
        ASSERT_EQ(var_u16->dims[0], 4);
        ASSERT_EQ(var_u16->dims[1], 2);
        ADIOS_VARINFO *var_u32 = adios_inq_var(f, "u32");
        ASSERT_NE(var_u32, nullptr);
        ASSERT_EQ(var_u32->ndim, 2);
        ASSERT_EQ(var_u32->dims[0], 4);
        ASSERT_EQ(var_u32->dims[1], 2);
        ADIOS_VARINFO *var_u64 = adios_inq_var(f, "u64");
        ASSERT_NE(var_u64, nullptr);
        ASSERT_EQ(var_u64->ndim, 2);
        ASSERT_EQ(var_u64->dims[0], 4);
        ASSERT_EQ(var_u64->dims[1], 2);
        ADIOS_VARINFO *var_r32 = adios_inq_var(f, "r32");
        ASSERT_NE(var_r32, nullptr);
        ASSERT_EQ(var_r32->ndim, 2);
        ASSERT_EQ(var_r32->dims[0], 4);
        ASSERT_EQ(var_r32->dims[1], 2);
        ADIOS_VARINFO *var_r64 = adios_inq_var(f, "r64");
        ASSERT_NE(var_r64, nullptr);
        ASSERT_EQ(var_r64->ndim, 2);
        ASSERT_EQ(var_r64->dims[0], 4);
        ASSERT_EQ(var_r64->dims[1], 2);

        std::array<char, 8> I8;
        std::array<int16_t, 8> I16;
        std::array<int32_t, 8> I32;
        std::array<int64_t, 8> I64;
        std::array<unsigned char, 8> U8;
        std::array<uint16_t, 8> U16;
        std::array<uint32_t, 8> U32;
        std::array<uint64_t, 8> U64;
        std::array<float, 8> R32;
        std::array<double, 8> R64;

        uint64_t start[2] = {0, 0};
        uint64_t count[2] = {4, 2};
        ADIOS_SELECTION *sel = adios_selection_boundingbox(2, start, count);

        // Read stuff
        for (size_t t = 0; t < 3; ++t)
        {
            // Read the current step
            adios_schedule_read_byid(f, sel, var_i8->varid, t, 1, I8.data());
            adios_schedule_read_byid(f, sel, var_i16->varid, t, 1, I16.data());
            adios_schedule_read_byid(f, sel, var_i32->varid, t, 1, I32.data());
            adios_schedule_read_byid(f, sel, var_i64->varid, t, 1, I64.data());
            adios_schedule_read_byid(f, sel, var_u8->varid, t, 1, U8.data());
            adios_schedule_read_byid(f, sel, var_u16->varid, t, 1, U16.data());
            adios_schedule_read_byid(f, sel, var_u32->varid, t, 1, U32.data());
            adios_schedule_read_byid(f, sel, var_u64->varid, t, 1, U64.data());
            adios_schedule_read_byid(f, sel, var_r32->varid, t, 1, R32.data());
            adios_schedule_read_byid(f, sel, var_r64->varid, t, 1, R64.data());
            adios_perform_reads(f, 1);

            // Check if it's correct
            for (size_t i = 0; i < 8; ++i)
            {
                std::stringstream ss;
                ss << "t=" << t << " i=" << i;
                std::string msg = ss.str();

                EXPECT_EQ(I8[i], m_TestData.I8[i + t]) << msg;
                EXPECT_EQ(I16[i], m_TestData.I16[i + t]) << msg;
                EXPECT_EQ(I32[i], m_TestData.I32[i + t]) << msg;
                EXPECT_EQ(I64[i], m_TestData.I64[i + t]) << msg;
                EXPECT_EQ(U8[i], m_TestData.U8[i + t]) << msg;
                EXPECT_EQ(U16[i], m_TestData.U16[i + t]) << msg;
                EXPECT_EQ(U32[i], m_TestData.U32[i + t]) << msg;
                EXPECT_EQ(U64[i], m_TestData.U64[i + t]) << msg;
                EXPECT_EQ(R32[i], m_TestData.R32[i + t]) << msg;
                EXPECT_EQ(R64[i], m_TestData.R64[i + t]) << msg;
            }
        }

        adios_selection_delete(sel);

        // Cleanup variable structures
        adios_free_varinfo(var_i8);
        adios_free_varinfo(var_i16);
        adios_free_varinfo(var_i32);
        adios_free_varinfo(var_i64);
        adios_free_varinfo(var_u8);
        adios_free_varinfo(var_u16);
        adios_free_varinfo(var_u32);
        adios_free_varinfo(var_u64);
        adios_free_varinfo(var_r32);
        adios_free_varinfo(var_r64);

        // Cleanup file
        adios_read_close(f);

        adios_read_finalize_method(ADIOS_READ_METHOD_BP);
    }
}

// ADIOS2 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS2ADIOS1WriteADIOS2ADIOS1Read2D4x2)
{
    std::string fname = "ADIOS2ADIOS1WriteADIOS2ADIOS1Read2D4x2Test.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

// Native ADIOS1 write, ADIOS2 read
TEST_F(ADIOS1WriteReadTest, DISABLED_ADIOS1WriteADIOS2ADIOS1Read2D4x2)
{
    std::string fname = "ADIOS1WriteADIOS2ADIOS1Read2D4x2Test.bp";

    ASSERT_TRUE(false) << "ADIOS2 read API is not yet implemented";
}

//******************************************************************************
// main
//******************************************************************************

int main(int argc, char **argv)
{
#ifdef ADIOS2_HAVE_MPI
    MPI_Init(nullptr, nullptr);
#endif

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

#ifdef ADIOS2_HAVE_MPI
    MPI_Finalize();
#endif

    return result;
}
