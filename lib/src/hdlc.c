/*
 * Copyright (c) 2025, Open Pixel Systems
 *
 * SPDX-License-Identifier: MIT
 */

#include "hdlc.h"

#include <string.h>

// Define HDLC_LOG_ENABLED to enable logging
//--------------------------------------------------
#ifdef HDLC_LOG_ENABLED
#include <stdio.h>
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#else
#define ERR(...)
#endif

// Defines used in HDLC encoding/decoding
//--------------------------------------------------
#define HDLC_DELIMITER 0x7E
#define HDLC_ESCAPE    0x7D
#define HDLC_INVERTED  0x20

// Defines used in CRC calculation
//--------------------------------------------------
#define CRC_POLY    0x1021
#define CRC_INIT    0xFFFF
#define CRC_XOR_OUT 0xFFFF

// Macros used in HDLC encoding/decoding
//--------------------------------------------------
#define LOW_BYTE(x)  ((x) & 0xFF)
#define HIGH_BYTE(x) (((x) >> 8) & 0xFF)

//--------------------------------------------------
static int _hdlc_write_byte(uint8_t byte, uint8_t *data, int len)
{
	// Check if the byte needs to be escaped
	if (byte == HDLC_DELIMITER || byte == HDLC_ESCAPE) {
		// Check if there is enough space in the buffer
		if (len < 2) {
			ERR("[%s:%d] len < 2\n", __func__, __LINE__);
			return -1;
		}

		// Add the escape byte
		*data++ = HDLC_ESCAPE;

		// Copy the data byte with the inverted bit (to indicate escaping)
		*data++ = byte ^ HDLC_INVERTED;

		return 2;
	} else {
		// Check if there is enough space in the buffer
		if (len < 1) {
			ERR("[%s:%d] len < 1\n", __func__, __LINE__);
			return -1;
		}

		// Copy the byte
		*data++ = byte;

		return 1;
	}
}

//--------------------------------------------------
static int _hdlc_read_byte(uint8_t *byte, uint8_t *data, int len)
{
	// Check if the byte needs to be unescaped
	if (*data == HDLC_ESCAPE) {
		// Check if there is enough space in the buffer
		if (len < 1) {
			ERR("[%s:%d] len < 1\n", __func__, __LINE__);
			return -1;
		}

		// Unescape the data byte (remove the inverted bit) and copy it
		*byte = *(++data) ^ HDLC_INVERTED;

		return 2;
	} else {
		// Check if there is enough space in the buffer
		if (len < 1) {
			ERR("[%s:%d] len < 1\n", __func__, __LINE__);
			return -1;
		}

		// Normal byte, copy the data byte
		*byte = *data;

		return 1;
	}
}

//--------------------------------------------------
static uint8_t _reverse_bits(uint8_t byte)
{
	// Reverse the bits in a byte
	byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
	byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
	byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
	return byte;
}

//--------------------------------------------------
static uint16_t _reverse_bits_16(uint16_t value)
{
	// Reverse the bits in a 16-bit value
	value = (value & 0xFF00) >> 8 | (value & 0x00FF) << 8;
	value = (value & 0xF0F0) >> 4 | (value & 0x0F0F) << 4;
	value = (value & 0xCCCC) >> 2 | (value & 0x3333) << 2;
	value = (value & 0xAAAA) >> 1 | (value & 0x5555) << 1;
	return value;
}

// CRC-16/ISO-HDLC: x^16 + x^12 + x^5 + 1 (0x1021)
//--------------------------------------------------
static uint16_t _hdlc_calculate_fcs(uint8_t *data, int len)
{
	// Set the initial FCS value
	uint16_t fcs = CRC_INIT;

	// Calculate the FCS
	while (len--) {
		// Reverse the bits in the data byte
		fcs ^= (_reverse_bits(*data++) << 8);

		// Calculate the FCS for each bit in the data byte
		for (int i = 0; i < 8; i++) {
			// Check if the current bit is 1
			if (fcs & 0x8000) {
				// XOR the FCS with the polynomial, then shift the FCS to the left
				fcs = (fcs << 1) ^ CRC_POLY;
			} else {
				// Shift the FCS to the left
				fcs <<= 1;
			}
		}
	}

	// Reverse the bits in the FCS
	fcs = _reverse_bits_16(fcs);

	// XOR the FCS with the final XOR value
	return fcs ^ CRC_XOR_OUT;
}

//--------------------------------------------------
int hdlc_frame_init(hdlc_frame_t *frame)
{
	// Check for NULL pointer
	if (frame == NULL) {
		// Initialization failed, return -1
		ERR("[%s:%d] frame == NULL\n", __func__, __LINE__);
		return -1;
	}

	// Initialize the HDLC frame with zeros
	memset(frame, 0, sizeof(*frame));

	// Return success
	return 0;
}

//--------------------------------------------------
int hdlc_i_frame_control_init(hdlc_control_t *control, uint8_t ns, uint8_t pf, uint8_t nr)
{
	// Check for NULL pointer
	if (control == NULL) {
		// Initialization failed, return -1
		ERR("[%s:%d] control == NULL\n", __func__, __LINE__);
		return -1;
	}

	// Initialize the I-frame control field
	control->i_fields.res1 = 1;
	control->i_fields.ns = ns;
	control->i_fields.pf = pf;
	control->i_fields.nr = nr;

	// Return success
	return 0;
}

//--------------------------------------------------
int hdlc_s_frame_control_init(hdlc_control_t *control, hdlc_control_s_frame_code_t s, uint8_t pf,
			      uint8_t nr)
{
	// Check for NULL pointer
	if (control == NULL) {
		// Initialization failed, return -1
		ERR("[%s:%d] control == NULL\n", __func__, __LINE__);
		return -1;
	}

	// Initialize the S-frame control field
	control->s_fields.res1 = 1;
	control->s_fields.res2 = 0;
	control->s_fields.s = s;
	control->s_fields.pf = pf;
	control->s_fields.nr = nr;

	// Return success
	return 0;
}

//--------------------------------------------------
int hdlc_u_frame_control_init(hdlc_control_t *control, hdlc_control_u_frame_code_t m, uint8_t pf)
{
	// Check for NULL pointer
	if (control == NULL) {
		// Initialization failed, return -1
		ERR("[%s:%d] control == NULL\n", __func__, __LINE__);
		return -1;
	}

	// Initialize the U-frame control field
	control->u_fields.res1 = 1;
	control->u_fields.res2 = 1;
	control->u_fields.pf = pf;

	// Set the U-frame M1 and M2 fields
	switch (m) {
	case HDLC_CONTROL_U_FRAME_CODE_SNRM:
		control->u_fields.m1 = 0b00;
		control->u_fields.m2 = 0b001;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_SABM:
		control->u_fields.m1 = 0b11;
		control->u_fields.m2 = 0b100;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_SABME:
		control->u_fields.m1 = 0b11;
		control->u_fields.m2 = 0b110;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_DISC:
		control->u_fields.m1 = 0b00;
		control->u_fields.m2 = 0b010;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_UA:
		control->u_fields.m1 = 0b00;
		control->u_fields.m2 = 0b110;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_RSET:
		control->u_fields.m1 = 0b11;
		control->u_fields.m2 = 0b001;
		break;
	case HDLC_CONTROL_U_FRAME_CODE_FRMR:
		control->u_fields.m1 = 0b10;
		control->u_fields.m2 = 0b001;
		break;
	default:
		// Initialization failed, return -1
		ERR("[%s:%d] Unknown U-frame code\n", __func__, __LINE__);
		return -1;
	}

	// Return success
	return 0;
}

//--------------------------------------------------
int hdlc_encode(const hdlc_frame_t *frame, uint8_t *data, int len)
{
	// Check for NULL pointers
	if (frame == NULL || data == NULL) {
		// Encoding failed, return -1
		ERR("[%s:%d] frame == NULL || data == NULL\n", __func__, __LINE__);
		return -1;
	}

	int encoded_len = 0;
	int data_len = len;
	int result = 0;

	hdlc_info_len_t info_len = frame->info_len;
	size_t info_byte_index = 0;

	// Check if the len is valid
	if (data_len < 1) {
		// Encoding failed, return -1
		ERR("[%s:%d] data_len == 0\n", __func__, __LINE__);
		return -1;
	}

	// Add the start flag
	data[encoded_len++] = HDLC_DELIMITER;

	// Add the address
	result = _hdlc_write_byte(frame->address, data + encoded_len, len--);
	if (result < 1) {
		// Encoding failed, return -1
		ERR("[%s:%d] result < 1\n", __func__, __LINE__);
		return -1;
	}

	encoded_len += result;

	// Add the control field
	result = _hdlc_write_byte(frame->control.value, data + encoded_len, len--);
	if (result < 1) {
		// Encoding failed, return -1
		ERR("[%s:%d] result < 1\n", __func__, __LINE__);
		return -1;
	}

	encoded_len += result;

	// Skip the information field if it is empty
	if (info_len != 0) {
		while (info_len--) {
			// Set the current information byte
			const uint8_t info_byte = frame->info[info_byte_index++];

			// Add the information byte
			result = _hdlc_write_byte(info_byte, data + encoded_len, len--);
			if (result < 1) {
				// Encoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			encoded_len += result;
		}
	}

	// Check if there is enough space in the buffer
	if (encoded_len == 0) {
		// Encoding failed, return -1
		ERR("[%s:%d] encoded_len == 0\n", __func__, __LINE__);
		return -1;
	}

	// Calculate the FCS
	const uint16_t fcs = _hdlc_calculate_fcs(data + 1, encoded_len - 1);

	// Add the FCS (high byte)
	result = _hdlc_write_byte(HIGH_BYTE(fcs), data + encoded_len, len--);
	if (result < 1) {
		// Encoding failed, return -1
		ERR("[%s:%d] result < 1\n", __func__, __LINE__);
		return -1;
	}

	encoded_len += result;

	// Add the FCS (low byte)
	result = _hdlc_write_byte(LOW_BYTE(fcs), data + encoded_len, len--);
	if (result < 1) {
		// Encoding failed, return -1
		ERR("[%s:%d] result < 1\n", __func__, __LINE__);
		return -1;
	}

	encoded_len += result;

	// Check if there is enough space in the buffer
	if (encoded_len >= data_len) {
		// Encoding failed, return -1
		ERR("[%s:%d] encoded_len >= data_len\n", __func__, __LINE__);
		return -1;
	}

	// Add the stop flag
	data[encoded_len++] = HDLC_DELIMITER;

	// Return the encoded length
	return encoded_len;
}

//--------------------------------------------------
int hdlc_decode(hdlc_frame_t *frame, uint8_t *data, int len)
{
	// Check for NULL pointers
	if (frame == NULL || data == NULL) {
		// Decoding failed, return -1
		ERR("[%s:%d] frame == NULL || data == NULL\n", __func__, __LINE__);
		return -1;
	}

	// Initialize the HDLC state
	hdlc_state_t state = HDLC_STATE_START_FLAG;

	int result = 0;

	uint16_t fcs = 0;

	// Loop through the data buffer
	for (int i = 0; i < len; i++) {
		const int left = len - i;

		switch (state) {
		case HDLC_STATE_START_FLAG:
			// Check for the start flag
			if (data[i] == HDLC_DELIMITER) {
				// Transition to the address state
				state = HDLC_STATE_ADDRESS;
			} else {
				// Decoding failed, return -1
				ERR("[%s:%d] HDLC_DELIMITER error\n", __func__, __LINE__);
				return -1;
			}
			break;
		case HDLC_STATE_ADDRESS:
			// Read the address field
			result = _hdlc_read_byte(&frame->address, data + i, left);
			if (result < 1) {
				// Decoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			i += result - 1;

			// Transition to the control state
			state = HDLC_STATE_CONTROL;
			break;
		case HDLC_STATE_CONTROL:
			// Read the control field
			result = _hdlc_read_byte(&frame->control.value, data + i, left);
			if (result < 1) {
				// Decoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			i += result - 1;

			// Transition to the information state
			state = HDLC_STATE_INFO;
			break;
		case HDLC_STATE_INFO:
			// Check if all the information field bytes have been read (if any)
			if ((len - i) <= 3) { // 2 bytes for FCS and 1 byte for stop flag
				// Re-adjust the index to decode the FCS (current byte is part of
				// the FCS)
				i--;

				// Transition to the FCS state
				state = HDLC_STATE_FCS;
				break;
			}

			// Read the information field
			result = _hdlc_read_byte(&frame->info[frame->info_len], data + i, left);
			if (result < 1) {
				// Decoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			i += result - 1;

			// Increment the information field length
			frame->info_len++;
			break;
		case HDLC_STATE_FCS:
			uint8_t fcs_high = 0;
			uint8_t fcs_low = 0;

			// Read the FCS (high byte)
			result = _hdlc_read_byte(&fcs_high, data + i, left);
			if (result < 1) {
				// Decoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			i += result;

			// Read the FCS (low byte)
			result = _hdlc_read_byte(&fcs_low, data + i, left);
			if (result < 1) {
				// Decoding failed, return -1
				ERR("[%s:%d] result < 1\n", __func__, __LINE__);
				return -1;
			}

			i += result - 1;

			// Combine the FCS bytes
			fcs = (fcs_high << 8) | fcs_low;

			// Check the calculated FCS with the decoded FCS
			if (_hdlc_calculate_fcs(data + 1, len - 4) == fcs) {
				// Transition to the stop flag state
				state = HDLC_STATE_STOP_FLAG;
			} else {
				// Decoding failed, return -1
				ERR("[%s:%d] FCS error\n", __func__, __LINE__);
				return -1;
			}
			break;
		case HDLC_STATE_STOP_FLAG:
			// Check for the stop flag
			if (data[0] == HDLC_DELIMITER) {
				// Decoding successful, return success
				return 0;
			}
			break;
		default:
			// Decoding failed, return -1
			ERR("[%s:%d] Unknown state\n", __func__, __LINE__);
			return -1;
		}
	}

	// Decoding failed, return -1
	ERR("[%s:%d] No stop flag detected\n", __func__, __LINE__);
	return -1;
}