/**
 * Copyright (c) 2010 William Light <wrl@illest.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"

int monome_platform_open(monome_t *monome, const char *dev) {
	struct termios nt, ot;
	int fd;

	if( (fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0 ) {
		perror("libmonome: could not open monome device");
		return 1;
	}

	tcgetattr(fd, &ot);
	nt = ot;

	/* baud rate */
	cfsetispeed(&nt, B115200);
	cfsetospeed(&nt, B115200);

	/* parity (8N1) */
	nt.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	nt.c_cflag |=  (CS8 | CLOCAL | CREAD);

	/* no line processing */
	nt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);

	/* raw input */
	nt.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK |
	                INPCK | ISTRIP | IXON);

	/* raw output */
	nt.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR |
	                OFILL | OPOST);

	/* block until one character is read */
	nt.c_cc[VMIN]  = 1;
	nt.c_cc[VTIME] = 0;

	if( tcsetattr(fd, TCSANOW, &nt) < 0 ) {
		perror("libmonome: could not set terminal attributes");
		return 1;
	}

	tcflush(fd, TCIOFLUSH);

	monome->fd = fd;
	monome->ot = ot;

	return 0;
}

int monome_platform_close(monome_t *monome) {
	tcsetattr(monome->fd, TCSANOW, &monome->ot);
	return close(monome->fd);
}

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	ssize_t ret = write(monome->fd, buf, bufsize);

	if( ret < bufsize )
		perror("libmonome: write is missing bytes");

	if( ret < 0 )
		perror("libmonome: error in write");

	if( tcdrain(monome->fd) < 0 )
		perror("libmonome: error in tcdrain");

	return ret;
}

ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	return read(monome->fd, buf, count);
}
