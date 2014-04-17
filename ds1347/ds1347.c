/*
 * Dallas Maxim RTC DS1347 Driver
 *
 * Copyright (C) 2014 Raghavendra Chandra Ganiga <ravi23ganiga@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <rtc.h>
#include <spi.h>

#if defined(CONFIG_CMD_DATE)

/*
* DS1347 Registers
*/
#define RTC_SECONDS_REG		0x01
#define RTC_MINUTES_REG		0x03
#define RTC_HOURS_REG		0x05
#define RTC_DATE_REG		0x07
#define RTC_MONTH_REG		0x09
#define RTC_DAY_REG		0x0B
#define RTC_YEAR_REG		0x0D
#define RTC_CONTROL_REG		0x0F
#define RTC_CENTURY_REG		0x13
#define RTC_ALARM_CONFIG_REG	0x15
#define RTC_STATUS_REG		0x17
#define RTC_ALARM_SEC_REG	0x19
#define RTC_ALARM_MIN_REG	0x1B
#define RTC_ALARM_HOUR_REG	0x1D
#define RTC_ALARM_DATE_REG	0x1F
#define RTC_ALARM_MONTH_REG	0x21
#define RTC_ALARM_DAY_REG	0x23
#define RTC_ALARM_YEAR_REG	0x25
#define RTC_BURST_REG		0x3F

static struct spi_slave *slave;

static unsigned char rtc_read(unsigned char address);
static int rtc_write(unsigned char address, unsigned char value);

int rtc_get(struct rtc_time *tm)
{
	int ret;
	unsigned char data, century;
	if (!slave) {
		slave = spi_setup_slave(CONFIG_DS1347_BUS,
					CONFIG_DS1347_CS, 1000000,
					SPI_MODE_3);
		if (!slave)
			return -1;
	}

	ret = spi_claim_bus(slave);
	if (ret)
		return -1;

	data = rtc_read(RTC_SECONDS_REG);
	tm->tm_sec	= bcd2bin(data);

	data = rtc_read(RTC_MINUTES_REG);
	tm->tm_min	= bcd2bin(data);

	data = rtc_read(RTC_HOURS_REG);
	tm->tm_hour	= bcd2bin(data);

	data = rtc_read(RTC_DATE_REG);
	tm->tm_mday	= bcd2bin(data);

	data = rtc_read(RTC_MONTH_REG);
	tm->tm_mon	= bcd2bin(data);

	/* use the century register support in rtc */
	century = rtc_read(RTC_CENTURY_REG);
	data = rtc_read(RTC_YEAR_REG);
	tm->tm_year	= bcd2bin(data) + (bcd2bin(century) * 100);

	data = rtc_read(RTC_DAY_REG);
	tm->tm_wday	= bcd2bin(data) - 1;

	tm->tm_yday	= 0;
	tm->tm_isdst	= 0;

	spi_release_bus(slave);

	return 0;
}

int rtc_set(struct rtc_time *tm)
{
	int ret;
	unsigned char data;

	if (!slave) {
		slave = spi_setup_slave(CONFIG_DS1347_BUS,
					CONFIG_DS1347_CS, 1000000,
					SPI_MODE_3);

		if (!slave)
			return -1;
	}

	ret = spi_claim_bus(slave);
	if (ret)
		return -1;

	rtc_write(RTC_SECONDS_REG, bin2bcd(tm->tm_sec));

	rtc_write(RTC_MINUTES_REG, bin2bcd(tm->tm_min));

	rtc_write(RTC_HOURS_REG, bin2bcd(tm->tm_hour));

	rtc_write(RTC_DATE_REG, bin2bcd(tm->tm_mday));

	rtc_write(RTC_MONTH_REG, bin2bcd(tm->tm_mon));

	/* use the century register support in rtc */
	data = tm->tm_year / 100;
	rtc_write(RTC_CENTURY_REG, bin2bcd(data));

	data = tm->tm_year % 100;
	rtc_write(RTC_YEAR_REG, bin2bcd(data));

	rtc_write(RTC_DAY_REG, bin2bcd(tm->tm_wday + 1));

	spi_release_bus(slave);

	return 0;
}

void rtc_reset(void)
{
	int ret;

	if (!slave) {
		slave = spi_setup_slave(CONFIG_DS1347_BUS,
					CONFIG_DS1347_CS, 1000000,
					SPI_MODE_3);

		if (!slave)
			return;
	}

	ret = spi_claim_bus(slave);
	if (ret)
		return;

	/* Disable Write Protection */
	ret = rtc_write(RTC_CONTROL_REG, 0x00);

	/* Disable RTC Alarm - no alarm support */
	ret = rtc_write(RTC_ALARM_CONFIG_REG, 0x00);

	/* Enable Oscillator and No Glitch Filter */
	ret = rtc_write(RTC_STATUS_REG, 0x00);

	spi_release_bus(slave);
}
/*
* Helper Functions
*/
static unsigned char rtc_read(unsigned char address)
{
	address |= 0x80;

	return spi_w8r8(slave, address);
}

static int rtc_write(unsigned char address, unsigned char value)
{
	unsigned char dout[2];

	dout[0] = address;
	dout[1] = value;

	return spi_xfer(slave, 16, dout, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
}

#endif /* CONFIG_CMD_DATE */
