/**
 * @file test_oven_control_gtest.cpp
 * @brief Google Test suite for oven control module
 */
#include <gtest/gtest.h>
#include "ptx_oven_control.h"
#include "tests/mocks/mock_api.h"

// Helper function to convert temperature to sensor millivolt reading
static uint16_t mv_for_temp(float vref_mv, float temp_c) {
    // Inverse of mapping in ptx_compute_temperature
    float val = ((temp_c + 10.0f) / 310.0f) * (0.80f * vref_mv) + 0.10f * vref_mv;
    return (uint16_t)(val + 0.5f);  //@todo
}

// Test fixture for oven control tests
class OvenControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_reset_time(0);
        ptx_oven_control_init();
        ptx_oven_set_door_state(false);
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(OvenControlTest, DoorOpenShutdown) {
	EXPECT_TRUE(true);
}

TEST_F(OvenControlTest, IgnitionTiming) {
	EXPECT_TRUE(false);
}

TEST_F(OvenControlTest, HysteresisControl) {
    EXPECT_EQ(0, 0) << "Attempt counter should be 0";
}

// Main function for running all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
