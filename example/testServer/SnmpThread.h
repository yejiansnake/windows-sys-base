#pragma once

#include "common.h"

class CSnmpThread : public SysBase::CCustomThread, public SysBase::CSingletonT<CSnmpThread>
{
public:

	enum SNMP_TYPE
	{
		SNMP_TYPE_RECV = 0,
		SNMP_TYPE_SEND,

		SNMP_TYPE_MAX,
	};

	virtual ~CSnmpThread();
	friend class SysBase::CSingletonT<CSnmpThread>;

	void Add(SNMP_TYPE snmpType);

protected:

	virtual void Proc();

private:

	CSnmpThread();
	
	LONG m_Snmp[SNMP_TYPE_MAX];
};