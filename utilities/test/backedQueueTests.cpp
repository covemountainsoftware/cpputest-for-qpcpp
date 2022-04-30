/// @brief Trivial tests for the various utilities for QEQueue.
/// @ingroup
/// @cond
///***************************************************************************
///
/// Copyright (C) 2022 Matthew Eshleman. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, upon written permission from Matthew Eshleman, this program
/// may be distributed and modified under the terms of a Commercial
/// License. For further details, see the Contact Information below.
///
/// Contact Information:
///   Matthew Eshleman
///   https://covemountainsoftware.com
///   info@covemountainsoftware.com
///***************************************************************************
/// @endcond

#include "CppUTest/TestHarness.h"
#include "cmsVectorBackedQEQueue.hpp"
#include "cmsArrayBackedQEQueue.hpp"
#include <memory>

using namespace cms;

TEST_GROUP(BackedQueueTests) {void setup() final {} void teardown() final {}};

TEST(BackedQueueTests, can_create_vector_backed_qequeue)
{
    auto underTest =
      std::unique_ptr<VectorBackedQEQueue>(new VectorBackedQEQueue(10));
}

TEST(BackedQueueTests,
     vector_backed_qequeue_reports_size_plus_one_same_as_qequeue)
{
    constexpr uint16_t MaxStorageForEvents = 10;
    auto underTest = std::unique_ptr<VectorBackedQEQueue>(
      new VectorBackedQEQueue(MaxStorageForEvents));

    // see QEQueue docs for reason of +1
    CHECK_EQUAL(MaxStorageForEvents + 1, underTest->capacity());
}

TEST(BackedQueueTests, can_create_array_backed_qequeue)
{
    auto underTest =
      std::unique_ptr<ArrayBackedQEQueue<10>>(new ArrayBackedQEQueue<10>());
}

TEST(BackedQueueTests,
     array_backed_qequeue_reports_size_plus_one_same_as_qequeue)
{
    constexpr uint16_t MaxStorageForEvents = 10;
    auto underTest = std::unique_ptr<ArrayBackedQEQueue<MaxStorageForEvents>>(
      new ArrayBackedQEQueue<MaxStorageForEvents>());
    // see QEQueue docs for reason of +1
    CHECK_EQUAL(MaxStorageForEvents + 1, underTest->capacity());
}

TEST(BackedQueueTests, can_create_static_allocated_array_backed_qequeue)
{
    constexpr uint16_t MaxStorageForEvents = 22;
    using ArrayTestType = ArrayBackedQEQueue<MaxStorageForEvents>;
    static ArrayTestType underTest;
    // see QEQueue docs for reason of +1
    CHECK_EQUAL(MaxStorageForEvents + 1, underTest.capacity());
}

TEST(BackedQueueTests, can_push_up_to_max_events)
{
    constexpr uint16_t MaxStorageForEvents = 2;
    using ArrayTestType = ArrayBackedQEQueue<MaxStorageForEvents>;
    auto underTest      = std::unique_ptr<ArrayTestType>(new ArrayTestType());

    // fill up the queue
    static QP::QEvt testEvent {5, 0, 0};
    for (size_t i = 0; i < underTest->capacity(); ++i) {
        underTest->post(&testEvent, QP::QF_NO_MARGIN, 0);
    }

    // confirm full
    CHECK_FALSE(underTest->isEmpty());
    CHECK_EQUAL(0, underTest->getNFree());

    // empty the queue
    for (size_t i = 0; i < underTest->capacity(); ++i) {
        underTest->get(0);
    }

    // confirm empty
    CHECK_TRUE(underTest->isEmpty());
    CHECK_EQUAL(MaxStorageForEvents + 1, underTest->getNFree());
}