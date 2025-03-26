/*
 * Copyright (c) 2025, Open Pixel Systems
 *
 * SPDX-License-Identifier: MIT
 */

extern "C" {
#include <hdlc.h>
}

#include <gtest/gtest.h>

#include <array>

//--------------------------------------------------
bool operator==(const hdlc_frame_t &lhs, const hdlc_frame_t &rhs)
{
	if (lhs.address != rhs.address) {
		return false;
	}

	if (lhs.control.value != rhs.control.value) {
		return false;
	}

	if (lhs.info_len != rhs.info_len) {
		return false;
	}

	for (uint8_t i = 0; i < lhs.info_len; i++) {
		if (lhs.info[i] != rhs.info[i]) {
			return false;
		}
	}

	return true;
}

namespace
{
//--------------------------------------------------
hdlc_control_t createIFrameControl(uint8_t ns, uint8_t pf, uint8_t nr)
{
	hdlc_control_t control = {0};

	hdlc_i_frame_control_init(&control, ns, pf, nr);

	return control;
}

//--------------------------------------------------
hdlc_frame_t createEmptyFrame()
{
	hdlc_frame_t frame = {0};

	// Initialize the HDLC frames
	hdlc_frame_init(&frame);

	return frame;
}

//--------------------------------------------------
template <size_t T = 0>
hdlc_frame_t createFrame(const hdlc_control_t &control, uint8_t address,
			 const std::array<uint8_t, T> &info = {})
{
	hdlc_frame_t frame = {0};

	// Initialize the HDLC frames
	hdlc_frame_init(&frame);

	// Set the address
	frame.address = address;

	// Set the control field
	frame.control = control;

	// Set the information field
	for (size_t i = 0; i < info.size(); i++) {
		frame.info[i] = info[i];
	}
	frame.info_len = static_cast<uint8_t>(info.size());

	return frame;
}
} // namespace

//--------------------------------------------------
TEST(verify_encode_decode_normal, success)
{
	auto control = createIFrameControl(0x00, 0x01, 0x02);
	auto information = std::array<uint8_t, 4>{0x04, 0x05, 0x06, 0x07};

	hdlc_frame_t original_frame = createFrame(control, 0x03, information);
	hdlc_frame_t decoded_frame = createEmptyFrame();

	uint8_t buffer[64] = {0};

	const int buffer_len = hdlc_encode(&original_frame, buffer, sizeof(buffer));
	EXPECT_EQ(buffer_len, 10);

	const uint8_t buffer_check[] = {0x7E, 0x03, 0x51, 0x04, 0x05, 0x06, 0x07, 0xEE, 0xEA, 0x7E};

	for (int i = 0; i < buffer_len; i++) {
		EXPECT_EQ(buffer[i], buffer_check[i]);
	}

	const int decode_result = hdlc_decode(&decoded_frame, buffer, buffer_len);
	EXPECT_EQ(decode_result, 0);

	EXPECT_EQ(original_frame, decoded_frame);
}

//--------------------------------------------------
TEST(verify_encode_decode_escaped, success)
{
	auto control = createIFrameControl(0x7E, 0x7E, 0x7E);
	auto information = std::array<uint8_t, 4>{0x7E, 0x7E, 0x7E, 0x7E};

	hdlc_frame_t original_frame = createFrame(control, 0x7E, information);
	hdlc_frame_t decoded_frame = createEmptyFrame();

	uint8_t buffer[64] = {0};

	const int buffer_len = hdlc_encode(&original_frame, buffer, sizeof(buffer));
	EXPECT_EQ(buffer_len, 15);

	const uint8_t buffer_check[] = {0x7E, 0x7D, 0x5E, 0xCD, 0x7D, 0x5E, 0x7D, 0x5E,
					0x7D, 0x5E, 0x7D, 0x5E, 0x50, 0xFF, 0x7E};

	for (int i = 0; i < buffer_len; i++) {
		EXPECT_EQ(buffer[i], buffer_check[i]);
	}

	const int decode_result = hdlc_decode(&decoded_frame, buffer, buffer_len);
	EXPECT_EQ(decode_result, 0);

	EXPECT_EQ(original_frame, decoded_frame);
}

//--------------------------------------------------
TEST(verify_i_frame_control_init, success)
{
	hdlc_control_t control = {0};

	{
		// ns: 0b000, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b001, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x01, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x01);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b010, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x02, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x02);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b011, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x03, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x03);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b100, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x04, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x04);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b101, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x05, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x05);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b110, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x06, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x06);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b111, pf: 0b0, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x07, 0x00, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x07);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b000, pf: 0b1, nr: 0b000

		hdlc_i_frame_control_init(&control, 0x00, 0x01, 0x00);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x01);
		EXPECT_EQ(control.i_fields.nr, 0x00);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b001

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x01);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x01);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b010

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x02);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x02);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b011

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x03);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x03);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b100

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x04);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x04);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b101

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x05);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x05);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b110

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x06);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x06);
	}

	{
		// ns: 0b000, pf: 0b0, nr: 0b111

		hdlc_i_frame_control_init(&control, 0x00, 0x00, 0x07);

		EXPECT_EQ(control.i_fields.res1, 0x01);
		EXPECT_EQ(control.i_fields.ns, 0x00);
		EXPECT_EQ(control.i_fields.pf, 0x00);
		EXPECT_EQ(control.i_fields.nr, 0x07);
	}
}

//--------------------------------------------------
TEST(verify_s_frame_control_init, success)
{
	hdlc_control_t control = {0};

	{
		// s: 0b00, pf: 0b0, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x00);
	}

	{
		// s: RR, pf: 0b0, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x0);
	}

	{
		// s: REJ, pf: 0b0, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_REJ, 0x00, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x01);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x0);
	}

	{
		// s: RNR, pf: 0b0, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RNR, 0x00, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x02);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x0);
	}

	{
		// s: SREJ, pf: 0b0, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_SREJ, 0x00, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x03);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x0);
	}

	{
		// s: 0x00, pf: 0b1, nr: 0b000

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x01, 0x00);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x01);
		EXPECT_EQ(control.s_fields.nr, 0x0);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b001

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x01);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x1);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b010

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x02);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x2);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b011

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x03);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x3);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b100

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x04);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x4);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b101

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x05);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x5);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b110

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x06);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x6);
	}

	{
		// s: 0x00, pf: 0b0, nr: 0b111

		hdlc_s_frame_control_init(&control, HDLC_CONTROL_S_FRAME_CODE_RR, 0x00, 0x07);

		EXPECT_EQ(control.s_fields.res1, 0x01);
		EXPECT_EQ(control.s_fields.res2, 0x00);
		EXPECT_EQ(control.s_fields.s, 0x00);
		EXPECT_EQ(control.s_fields.pf, 0x00);
		EXPECT_EQ(control.s_fields.nr, 0x7);
	}
}

//--------------------------------------------------
TEST(verify_u_frame_control_init, success)
{
	hdlc_control_t control = {0};

	{
		// m: SNRM, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_SNRM, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x00);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x01);
	}

	{
		// m: SABM, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_SABM, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x03);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x04);
	}

	{
		// m: SABME, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_SABME, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x03);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x06);
	}

	{
		// m: DISC, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_DISC, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x00);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x02);
	}

	{
		// m: UA, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_UA, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x00);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x06);
	}

	{
		// m: RSET, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_RSET, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x03);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x01);
	}

	{
		// m: FRMR, pf: 0b0

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_FRMR, 0x00);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x02);
		EXPECT_EQ(control.u_fields.pf, 0x00);
		EXPECT_EQ(control.u_fields.m2, 0x01);
	}

	{
		// m: SNRM, pf: 0b1

		hdlc_u_frame_control_init(&control, HDLC_CONTROL_U_FRAME_CODE_SNRM, 0x01);

		EXPECT_EQ(control.u_fields.res1, 0x01);
		EXPECT_EQ(control.u_fields.res2, 0x01);
		EXPECT_EQ(control.u_fields.m1, 0x00);
		EXPECT_EQ(control.u_fields.pf, 0x01);
		EXPECT_EQ(control.u_fields.m2, 0x01);
	}
}

//--------------------------------------------------
TEST(verify_encode_buffer_len_check_with_frame_information_normal, success)
{
	auto control = createIFrameControl(0x00, 0x01, 0x02);
	auto information = std::array<uint8_t, 1>{0x04};

	hdlc_frame_t original_frame = createFrame(control, 0x03, information);

	uint8_t buffer[64] = {0};

	// Buffer is negative
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, -1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 0
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 0);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 1
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 2
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 2);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 3
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 3);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 4
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 4);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 5
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 5);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 6 (1 less than required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 6);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 7 (required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 7);
		EXPECT_EQ(buffer_len, 7);
	}
}

//--------------------------------------------------
TEST(verify_encode_buffer_len_check_without_frame_information_normal, success)
{
	auto control = createIFrameControl(0x00, 0x01, 0x02);

	hdlc_frame_t original_frame = createFrame(control, 0x03);

	uint8_t buffer[64] = {0};

	// Buffer is negative
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, -1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 0
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 0);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 1
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 2
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 2);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 3
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 3);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 4
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 4);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 5 (1 less than required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 5);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 6 (required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 6);
		EXPECT_EQ(buffer_len, 6);
	}
}

//--------------------------------------------------
TEST(verify_encode_buffer_len_check_with_frame_information_escaped, success)
{
	auto control = createIFrameControl(0x7E, 0x7E, 0x7E);
	auto information = std::array<uint8_t, 1>{0x7E};

	hdlc_frame_t original_frame = createFrame(control, 0x7E, information);

	uint8_t buffer[64] = {0};

	// Buffer is negative
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, -1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 0
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 0);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 1
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 2
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 2);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 3
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 3);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 4
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 4);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 5
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 5);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 6
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 6);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 7
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 7);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 8 (1 less than required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 8);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 9 (required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 9);
		EXPECT_EQ(buffer_len, 9);
	}
}

//--------------------------------------------------
TEST(verify_encode_buffer_len_check_without_frame_information, success)
{
	auto control = createIFrameControl(0x7E, 0x7E, 0x7E);

	hdlc_frame_t original_frame = createFrame(control, 0x7E);

	uint8_t buffer[64] = {0};

	// Buffer is negative
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, -1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 0
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 0);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 1
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 1);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 2
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 2);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 3
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 3);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 4
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 4);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 5
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 5);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 6 (1 less than required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 6);
		EXPECT_EQ(buffer_len, -1);
	}

	// Buffer is 7 (required)
	{
		const int buffer_len = hdlc_encode(&original_frame, buffer, 7);
		EXPECT_EQ(buffer_len, 7);
	}
}

//--------------------------------------------------
TEST(verify_decode_buffer_len_check_normal, success)
{
	auto control = createIFrameControl(0x00, 0x01, 0x02);
	auto information = std::array<uint8_t, 1>{0x04};

	hdlc_frame_t original_frame = createFrame(control, 0x03, information);
	hdlc_frame_t decoded_frame = createEmptyFrame();

	uint8_t buffer[64] = {0};

	const int buffer_len = hdlc_encode(&original_frame, buffer, sizeof(buffer));
	EXPECT_EQ(buffer_len, 7);

	// Buffer is negative
	{
		const int result = hdlc_decode(&decoded_frame, buffer, -1);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 0
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 0);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 1
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 1);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 2
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 2);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 3
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 3);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 4
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 4);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 5
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 5);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 6 (1 less than required)
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 6);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 7 (required)
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 7);
		EXPECT_EQ(result, 0);
	}
}

//--------------------------------------------------
TEST(verify_decode_buffer_len_check_escaped, success)
{
	auto control = createIFrameControl(0x7E, 0x7E, 0x7E);
	auto information = std::array<uint8_t, 1>{0x7E};

	hdlc_frame_t original_frame = createFrame(control, 0x7E, information);
	hdlc_frame_t decoded_frame = createEmptyFrame();

	uint8_t buffer[64] = {0};

	const int buffer_len = hdlc_encode(&original_frame, buffer, sizeof(buffer));
	EXPECT_EQ(buffer_len, 9);

	// Buffer is negative
	{
		const int result = hdlc_decode(&decoded_frame, buffer, -1);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 0
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 0);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 1
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 1);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 2
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 2);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 3
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 3);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 4
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 4);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 5
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 5);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 6
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 6);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 7
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 7);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 8 (1 less than required)
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 8);
		EXPECT_EQ(result, -1);
	}

	// Buffer is 9 (required)
	{
		const int result = hdlc_decode(&decoded_frame, buffer, 9);
		EXPECT_EQ(result, 0);
	}
}

//--------------------------------------------------
int main()
{
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}