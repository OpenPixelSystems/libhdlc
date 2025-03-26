/*
 * Copyright (c) 2025, Open Pixel Systems
 *
 * SPDX-License-Identifier: MIT
 */

#include <hdlc.h>

#include <stdio.h>
#include <string.h>

//--------------------------------------------------
static void print_frame(hdlc_frame_t *frame)
{
	printf("Address: 0x%02X, Control: 0x%02X", frame->address, frame->control.value);

	if (frame->info_len > 0) {
		printf(", Info: ");
	}

	for (uint8_t i = 0; i < frame->info_len; i++) {
		printf("0x%02X ", frame->info[i]);
	}

	printf("\n");
}

//--------------------------------------------------
static int compare_frames(hdlc_frame_t *frame1, hdlc_frame_t *frame2)
{
	return memcmp(frame1, frame2, sizeof(hdlc_frame_t));
}

//--------------------------------------------------
int main(void)
{
	printf("Basic example\n");

	const uint8_t send_sequence_number = 0x01;
	const uint8_t receive_sequence_number = 0x02;

	hdlc_frame_t original_frame = {0};
	hdlc_frame_t decoded_frame = {0};

	uint8_t buffer[64] = {0};

	// Initialize the HDLC frames
	hdlc_frame_init(&original_frame);
	hdlc_frame_init(&decoded_frame);

	// Start the encoding process

	original_frame.address = 0x03;

	// Initialize the I-frame control field
	hdlc_i_frame_control_init(&original_frame.control, send_sequence_number, 1,
				  receive_sequence_number);

	// Set the information field
	original_frame.info[0] = 0x04;
	original_frame.info[1] = 0x05;
	original_frame.info[2] = 0x06;
	original_frame.info[3] = 0x07;
	original_frame.info_len = 4;

	// Encode a frame
	const int buffer_len = hdlc_encode(&original_frame, buffer, sizeof(buffer));
	if (buffer_len < 0) {
		printf("Failed to encode frame\n");
		return 1;
	}

	// Start the decoding process

	// Decode a frame
	const int result = hdlc_decode(&decoded_frame, buffer, buffer_len);
	if (result < 0) {
		printf("Failed to decode frame\n");
		return 1;
	}

	printf("Original frame => ");
	print_frame(&original_frame);

	printf("Decoded frame  => ");
	print_frame(&decoded_frame);

	if (compare_frames(&original_frame, &decoded_frame) != 0) {
		printf("Decoded frame does not match original frame\n");
		return 1;
	}

	printf("Decoded frame matches original frame\n");
	return 0;
}