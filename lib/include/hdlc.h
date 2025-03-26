/*
 * Copyright (c) 2025, Open Pixel Systems
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef HDLC_INFO_MAX_LEN
#define HDLC_INFO_MAX_LEN 255
#endif

#if HDLC_INFO_MAX_LEN > 0xFF
#error "HDLC_INFO_MAX_LEN must be less than or equal to 255"
#endif

typedef uint8_t hdlc_address_t;
typedef uint8_t hdlc_info_t[HDLC_INFO_MAX_LEN];
typedef uint8_t hdlc_info_len_t;

typedef union {
	uint8_t value;
	struct {
		uint8_t res1: 1;
		uint8_t ns: 3;
		uint8_t pf: 1;
		uint8_t nr: 3;
	} __attribute__((packed)) i_fields;
	struct {
		uint8_t res1: 1;
		uint8_t res2: 1;
		uint8_t s: 2;
		uint8_t pf: 1;
		uint8_t nr: 3;
	} __attribute__((packed)) s_fields;
	struct {
		uint8_t res1: 1;
		uint8_t res2: 1;
		uint8_t m1: 2;
		uint8_t pf: 1;
		uint8_t m2: 3;
	} __attribute__((packed)) u_fields;
} __attribute__((packed)) hdlc_control_t;

typedef enum {
	HDLC_CONTROL_S_FRAME_CODE_RR = 0x00,  // Receive ready
	HDLC_CONTROL_S_FRAME_CODE_REJ = 0x01, // Reject on frame N(rR)
	HDLC_CONTROL_S_FRAME_CODE_RNR = 0x02, // Receive not ready
	HDLC_CONTROL_S_FRAME_CODE_SREJ = 0x03 // Selective reject on frame N(R)
} hdlc_control_s_frame_code_t;

typedef enum {
	HDLC_CONTROL_U_FRAME_CODE_SNRM,  // Set normal response mode
	HDLC_CONTROL_U_FRAME_CODE_SABM,  // Set asynchronous balanced mode
	HDLC_CONTROL_U_FRAME_CODE_SABME, // Set asynchronous balanced mode extended
	HDLC_CONTROL_U_FRAME_CODE_DISC,  // Disconnect
	HDLC_CONTROL_U_FRAME_CODE_UA,    // Un-numbered acknowledgement
	HDLC_CONTROL_U_FRAME_CODE_RSET,  // Reset send and receive sequence numbers
	HDLC_CONTROL_U_FRAME_CODE_FRMR,  // Frame reject
} hdlc_control_u_frame_code_t;

typedef enum {
	HDLC_STATE_IDLE,
	HDLC_STATE_START_FLAG,
	HDLC_STATE_ADDRESS,
	HDLC_STATE_CONTROL,
	HDLC_STATE_INFO,
	HDLC_STATE_FCS,
	HDLC_STATE_STOP_FLAG,
} hdlc_state_t;

typedef struct {
	hdlc_address_t address;
	hdlc_control_t control;
	hdlc_info_t info;
	hdlc_info_len_t info_len;
} __attribute__((packed)) hdlc_frame_t;

int hdlc_frame_init(hdlc_frame_t *frame);

void hdlc_i_frame_control_init(hdlc_control_t *control, uint8_t ns, uint8_t pf, uint8_t nr);
void hdlc_s_frame_control_init(hdlc_control_t *control, hdlc_control_s_frame_code_t s, uint8_t pf,
			       uint8_t nr);
int hdlc_u_frame_control_init(hdlc_control_t *control, hdlc_control_u_frame_code_t m, uint8_t pf);

int hdlc_encode(const hdlc_frame_t *frame, uint8_t *data, int len);
int hdlc_decode(hdlc_frame_t *frame, uint8_t *data, int len);