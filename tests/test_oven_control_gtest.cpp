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
    float x = (temp_c + 48.75f) / 387.5f;
    return (uint16_t)(x * vref_mv);  //@todo
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
	mock_set_vref_mv(5000);
    mock_set_signal_mv(mv_for_temp(5000, 160.0f)); // below ON threshold
    
    ptx_oven_control_update();
    const ptx_oven_status_t* st = ptx_oven_get_status();
    EXPECT_TRUE(st->gas_on) << "Gas should start ON when temp below threshold";
    EXPECT_TRUE(st->igniter_on) << "Igniter should start ON";

    // Open door -> immediate shutdown on next update
    ptx_oven_set_door_state(true);
    ptx_oven_control_update();
    st = ptx_oven_get_status();
    EXPECT_FALSE(st->gas_on) << "Gas should turn OFF on door open";
    EXPECT_FALSE(st->igniter_on) << "Igniter should turn OFF on door open";
    EXPECT_FALSE(mock_get_gas_output()) << "Hardware gas output should be OFF";
    EXPECT_FALSE(mock_get_igniter_output()) << "Hardware igniter output should be OFF";
}

TEST_F(OvenControlTest, IgnitionTiming) {
    mock_set_vref_mv(5000);
    mock_set_signal_mv(mv_for_temp(5000, 160.0f));
    
    ptx_oven_control_update();
    const ptx_oven_status_t* st = ptx_oven_get_status();
    EXPECT_TRUE(st->igniter_on) << "Igniter should be ON during ignition phase";

    mock_advance_ms(5000);
    ptx_oven_control_update();
    st = ptx_oven_get_status();
    EXPECT_TRUE(st->gas_on) << "Gas should stay ON after ignition";
    EXPECT_FALSE(st->igniter_on) << "Igniter should turn OFF after 5 seconds";
}

TEST_F(OvenControlTest, HysteresisControl) {
    mock_set_vref_mv(5000);

    // Start heating (below ON threshold: 175°C)
    mock_set_signal_mv(mv_for_temp(5000, 160.0f));
    ptx_oven_control_update();

    const ptx_oven_status_t* st = ptx_oven_get_status();
    EXPECT_TRUE(st->gas_on) << "Heating should start below ON threshold";

    // Wait for ignition phase to complete (5s)
    mock_advance_ms(5000);
    ptx_oven_control_update();
    st = ptx_oven_get_status();
    EXPECT_TRUE(st->gas_on) << "Gas should stay ON after ignition";
    EXPECT_FALSE(st->igniter_on) << "Igniter should turn OFF after ignition";

    // Move above OFF threshold (185°C) - need to fill filter with new values
    mock_set_signal_mv(mv_for_temp(5000, 186.0f));
    ptx_oven_control_update();

    st = ptx_oven_get_status();    
    EXPECT_FALSE(st->gas_on) << "Gas should turn OFF above OFF threshold";
    EXPECT_FALSE(st->igniter_on) << "Igniter should turn OFF above OFF threshold";
}

// Main function for running all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
