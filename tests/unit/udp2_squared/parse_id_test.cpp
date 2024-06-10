#include <gtest/gtest.h>

extern "C" {
    #include "udp_squared/protocol.h"
}

TEST(UDP2Test, TestIDParse) {
    uint8_t id = makeID(UDP2_C, OBC_NULL);
    uint8_t bottle, state;
    parseID(id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_C);
    ASSERT_EQ(state, OBC_NULL);

    id = makeID(UDP2_B, static_cast<payload_state_t>(1));
    parseID(id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_B);
    ASSERT_EQ(state, 1);
}