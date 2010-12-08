/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_CALENDAR_H
#define AX_CALENDAR_H

AX_BEGIN_NAMESPACE

enum { START_YEAR = 1901 };

class AX_API DateTime {
public:
	struct Data {
		ushort_t year;				// from 1901 to 2050
		ushort_t month;				// [ 1..12 ]
		ushort_t day;				// [ 1..31 ]
		ushort_t hour;				// [ 0..23 ]
		ushort_t minute;				// [ 0..59 ]
		ushort_t second;				// [ 0..59 ]
		ushort_t milliseconds;		// [ 0..999 ]

		// below is calculated out
		ushort_t dayOfWeek;			// [ 0..6 ], 0 is sunday

		// for Lunar Calendar
		ushort_t tiangan;			// [ 0..9 ]
		ushort_t dizhi;				// [ 0..11 ]
		ushort_t lunarYear;			// [ 1900 .. 2050 ]
		ushort_t lunarMonth;			// [ 1..12 ]
		bool leapMonth;			// true or false
		ushort_t lunarDay;			// [ 1..30 ]
		ushort_t solarTerm;			// 0 isn't a solar day, or is [ 1..24]

		// for sun and moon
		float sunAngle;			// in degree, 0~360
		float moonAngle;			// in degree, 0~360

		// total time in milliseconds from 1901
		ulonglong_t totaltime;
	};

	DateTime();
	~DateTime();

	void initSystemTime();
	void initSystemTime(uint_t start_time);

	void init(ushort_t year, ushort_t month, ushort_t day, ushort_t hour, ushort_t minute, ushort_t second, uint_t start_time);
	void init(ushort_t year, ushort_t month, ushort_t day, ushort_t hour, ushort_t minute, ushort_t second);
	void init(ushort_t hour, ushort_t minute, ushort_t second, uint_t startTime);
	void init(ushort_t hour, ushort_t minute, ushort_t second);

	Data getData();
#if 0
	Data getData(uint_t now_time);
#endif
	void update(int curtime);

public:
	// �ж�year�ǲ�������
	static bool isLeapYear(ushort_t year);

	// ����year, month, day��Ӧ�����ڼ� 1��1��1�� --- 65535��12��31��
	static ushort_t weekDay(ushort_t year, ushort_t month, ushort_t day);

	// ����year��month�µ����� 1��1�� --- 65535��12��
	static ushort_t monthDays(ushort_t year, ushort_t month);

	// ��������lunarYear������lunarMonth�µ����������lunarMonthΪ����
	// ����Ϊ�ڶ���lunarMonth�µ��������������Ϊ0
	//  1901��1��---2050��12��
	static int lunarMonthDays(ushort_t lunarYear, ushort_t lunarMonth);

	// ��������lunarYear���������
	//  1901��1��---2050��12��
	static ushort_t lunarYearDays(ushort_t lunarYear);

	// return lunar year's tiangan and dizhi
	static void lunarYearToTianganDizhi(ushort_t lunarYear, ushort_t &tiangan, ushort_t &dizhi);

	// ��������lunarYear��������·ݣ���û�з���0
	//  1901��1��---2050��12��
	static ushort_t getLeapMonth(ushort_t lunarYear);

	// ��iYear���ʽ������ɼ��귨��ʾ���ַ���
	static std::string formatLunarYear(ushort_t  year);

	// ��iMonth��ʽ���������ַ���
	static std::string formatMonth(ushort_t month, bool lunar = true);

	// ��iDay��ʽ���������ַ���
	static std::string formatLunarDay(ushort_t  day);

	// ���㹫���������ڼ���������  1��1��1�� --- 65535��12��31��
	static int calcDateDiff(ushort_t endYear, ushort_t endMonth, ushort_t endDay, ushort_t startYear = START_YEAR, ushort_t startMonth =1, ushort_t startDay =1);

	// ���㹫��iYear��iMonth��iDay�ն�Ӧ����������,���ض�Ӧ���������� 0-24
	// 1901��1��1��---2050��12��31��
	static ushort_t getLunarDate(ushort_t year, ushort_t month, ushort_t day, ushort_t &lunarYear, ushort_t &lunarMonth, ushort_t &lunarDay, bool &leapMonth);

protected:
	// �����1901��1��1�չ�span_days������������
	static void l_calcLunarDate(ushort_t &year, ushort_t &month ,ushort_t &day, bool &leapMonth, int span_days);

	// �����1901��1��1�չ�span_days������������
	static void l_calcDate(ushort_t &year, ushort_t &month ,ushort_t &day, int span_days);

	// ���㹫��iYear��iMonth��iDay�ն�Ӧ�Ľ��� 0-24��0���ǽ���
	static ushort_t l_getLunarHolDay(ushort_t year, ushort_t month, ushort_t day);

private:
	Data m_startData;
	uint_t m_startMilliseconds;
	Data m_curData;
	uint_t m_curMilliseconds;
};

AX_END_NAMESPACE

#endif // AX_CALENDAR_H
