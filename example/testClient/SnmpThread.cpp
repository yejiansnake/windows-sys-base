#include "SnmpThread.h"

CSnmpThread::CSnmpThread()
{

}

CSnmpThread::~CSnmpThread()
{

}

void CSnmpThread::Add(SNMP_TYPE snmpType)
{
	InterlockedIncrement(&m_Snmp[snmpType]);
}

void CSnmpThread::Proc()
{
	while (TRUE)
	{
		Sleep(60 * 1000);
		
		LONG aSnmp[SNMP_TYPE_MAX] = {0};
		
		for (int nIndex = 0; nIndex < SNMP_TYPE_MAX; ++nIndex)
		{
			aSnmp[nIndex] = InterlockedExchange(&m_Snmp[nIndex], 0);
		}
		
		printf("Recv:%ld, Send:%ld \r\n\n", aSnmp[SNMP_TYPE_RECV], aSnmp[SNMP_TYPE_SEND]);
	}
}