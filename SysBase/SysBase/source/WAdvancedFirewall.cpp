#include "../SysBase_Interface.h"
#include <comutil.h>
#include <atlcomcli.h>

#pragma comment(lib, "comsupp.lib")

namespace SysBase
{
    //////////////////////////////////////////////////////////////////////////

    void GetWAdvancedFirewallRuleInfo(TWAFRule& aRule, INetFwRule* FwRule)
    {
        long lVal = 0;
        CComBSTR comBstrTemp;
        VARIANT_BOOL bEnabled;
        NET_FW_RULE_DIRECTION fwDirection;
        NET_FW_ACTION fwAction;
       
        if (S_OK == FwRule->get_Name(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strName = comBstrTemp;
            }
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_Description(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strDesc = comBstrTemp;
            }
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_ApplicationName(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strAppName = comBstrTemp; 
            }
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_ServiceName(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strServiceName = comBstrTemp;
            }
        }

        if (S_OK == FwRule->get_Protocol(&lVal))
        {
            aRule.nProtocol = (INT32)lVal;

            switch(lVal)
            {
            case NET_FW_IP_PROTOCOL_TCP: 
            case NET_FW_IP_PROTOCOL_UDP: 
            default:
                break;
            }

            if(lVal != NET_FW_IP_VERSION_V4 && lVal != NET_FW_IP_VERSION_V6)
            {
                comBstrTemp.Empty();
                if (S_OK == FwRule->get_LocalPorts(&comBstrTemp))
                {
                    if (comBstrTemp.m_str)
                    {
                        aRule.strLocalPorts = comBstrTemp;
                    }
                }

                comBstrTemp.Empty();
                if (S_OK == FwRule->get_RemotePorts(&comBstrTemp))
                {
                    if (comBstrTemp.m_str)
                    {
                        aRule.strRemotePorts = comBstrTemp;
                    }
                }
            }
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_LocalAddresses(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strLocalAddr = comBstrTemp;
            }
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_RemoteAddresses(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strRemoteAddr = comBstrTemp;
            }
        }

        if (S_OK == FwRule->get_Profiles(&lVal))
        {
            aRule.nProfiles = (INT32)lVal;
        }

        if (S_OK == FwRule->get_Direction(&fwDirection))
        {
            switch(fwDirection)
            {
            case NET_FW_RULE_DIR_IN:
                aRule.eDirection = WAF_DIRECTION_TYPE_IN;
                break;
            case NET_FW_RULE_DIR_OUT:
                aRule.eDirection = WAF_DIRECTION_TYPE_OUT;
                break;
            default:
                break;
            }
        }

        if (S_OK == FwRule->get_Action(&fwAction))
        {
            switch(fwAction)
            {
            case NET_FW_ACTION_BLOCK:
                aRule.eAction = WAF_ACTION_TYPE_BLOCK;
                break;
            case NET_FW_ACTION_ALLOW:
                aRule.eAction = WAF_ACTION_TYPE_ALLOW;
                break;
            default:
                break;
            }
        }

        if (S_OK == FwRule->get_Enabled(&bEnabled))
        {
            aRule.bEnabled = bEnabled == VARIANT_FALSE ? false : true;
        }

        comBstrTemp.Empty();
        if (S_OK == FwRule->get_Grouping(&comBstrTemp))
        {
            if (comBstrTemp.m_str)
            {
                aRule.strGroup = comBstrTemp;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    CWAdvancedFirewall::CWAdvancedFirewall()
    {
        ::CoInitialize(NULL);

        m_pPolicy = NULL;

        HRESULT hr = S_OK;

        hr = CoCreateInstance(
            __uuidof(NetFwPolicy2), 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(INetFwPolicy2), 
            (void**)&m_pPolicy);
    }

    CWAdvancedFirewall::~CWAdvancedFirewall()
    {
        if (!m_pPolicy)
        {
            return;
        }

        m_pPolicy->Release();

        m_pPolicy = NULL;

        ::CoUninitialize();
    }

    bool CWAdvancedFirewall::GetStatus(EWAFProfileType type, bool& bEnabled)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        VARIANT_BOOL bVarEnabled = VARIANT_FALSE;

        if (S_OK != m_pPolicy->get_FirewallEnabled((NET_FW_PROFILE_TYPE2)type, &bVarEnabled))
        {
            return false;
        }

        if (VARIANT_FALSE == bVarEnabled)
        {
            bEnabled = false;
        }
        else
        {
            bEnabled = true;
        }

        return true;
    }

    bool CWAdvancedFirewall::SetStatus(EWAFProfileType type, bool bEnable)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        VARIANT_BOOL bVarEnabled = VARIANT_FALSE;

        if (bEnable)
        {
            bVarEnabled = VARIANT_TRUE;
        }

        if (S_OK != m_pPolicy->put_FirewallEnabled((NET_FW_PROFILE_TYPE2)type, bVarEnabled))
        {
            return false;
        }

        return true;
    }

    bool CWAdvancedFirewall::GetRuleAll(list<TWAFRule>& ruleList)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        CComVariant var;
        ULONG nFetched = 0;
        HRESULT hr = S_OK;
        IEnumVARIANT* pVariant = NULL;
        IUnknown* pEnumerator = NULL;
        INetFwRules* pFwRules = NULL;
        INetFwRule* pFwRule = NULL;

        if (S_OK != m_pPolicy->get_Rules(&pFwRules))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRules, pFwRules);

        // Iterate through all of the rules in pFwRules
        if (S_OK != pFwRules->get__NewEnum(&pEnumerator))
        {
            return false;
        }

        if (!pEnumerator)
        {
            return false;
        }

        hr = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void**)&pVariant);

        CREATE_AUTO_RELEASE_PTR(IEnumVARIANT, pVariant);

        while(SUCCEEDED(hr) && hr != S_FALSE)
        {
            var.Clear();
            hr = pVariant->Next(1, &var, &nFetched);

            if (S_FALSE != hr)
            {
                if (SUCCEEDED(hr))
                {
                    hr = var.ChangeType(VT_DISPATCH);
                }

                if (SUCCEEDED(hr))
                {
                    hr = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
                }

                CREATE_AUTO_RELEASE_PTR(INetFwRule, pFwRule);

                if (SUCCEEDED(hr))
                {
                    //È¡³öÊôÐÔ
                    TWAFRule aRule;
                    
                    GetWAdvancedFirewallRuleInfo(aRule, pFwRule);

                    ruleList.push_back(aRule);
                }
            }
        }
  
        return true;
    }

    bool CWAdvancedFirewall::AddRule(TWAFRule& aRule)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        INetFwRules* pFwRules = NULL;
        INetFwRule* pFwRule = NULL;

        if (S_OK != m_pPolicy->get_Rules(&pFwRules))
        {
            return false;
        }

        if (S_OK != CoCreateInstance(
            __uuidof(NetFwRule),    //CLSID of the class whose object is to be created
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(INetFwRule),   // Identifier of the Interface used for communicating with the object
            (void**)&pFwRule))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRule, pFwRule);
        
        pFwRule->put_Enabled(aRule.bEnabled ? VARIANT_TRUE : VARIANT_FALSE);

        switch (aRule.eAction)
        {
        case WAF_ACTION_TYPE_BLOCK:
            pFwRule->put_Action(NET_FW_ACTION_BLOCK);
            break;
        case WAF_ACTION_TYPE_ALLOW:
            pFwRule->put_Action(NET_FW_ACTION_ALLOW);
            break;
            default:
                return false;
        }

        switch (aRule.eDirection)
        {
        case WAF_DIRECTION_TYPE_IN:
            pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
            break;
        case WAF_DIRECTION_TYPE_OUT:
            pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
            break;
        default:
            break;
        }

        _bstr_t bstrName = aRule.strName.c_str();
        _bstr_t bstrDesc = aRule.strDesc.c_str();
        _bstr_t bstrGroup = aRule.strGroup.c_str();
        _bstr_t bstrAppName = aRule.strAppName.c_str();
        _bstr_t bstrLocalAddr = aRule.strLocalAddr.c_str();
        _bstr_t bstrLocalLPorts = aRule.strLocalPorts.c_str();
        _bstr_t bstrRemoteAddr = aRule.strRemoteAddr.c_str();
        _bstr_t bstrRemotePorts = aRule.strRemotePorts.c_str();
        _bstr_t bstrServiceName = aRule.strServiceName.c_str();

        pFwRule->put_Name(bstrName);

        if (bstrDesc.length() > 0)
        {
            pFwRule->put_Description(bstrDesc);
        }

        if (bstrGroup.length() > 0)
        {
            pFwRule->put_Grouping(bstrGroup);
        }

        if (bstrAppName.length() > 0)
        {
            pFwRule->put_ApplicationName(bstrAppName);
        }

        if (bstrServiceName.length() > 0)
        {
            pFwRule->put_ServiceName(bstrServiceName);
        }

        pFwRule->put_Protocol(aRule.nProtocol);

        pFwRule->put_LocalAddresses(bstrLocalAddr);

        pFwRule->put_LocalPorts(bstrLocalLPorts);

        pFwRule->put_RemoteAddresses(bstrRemoteAddr);

        pFwRule->put_RemotePorts(bstrRemotePorts);

        pFwRule->put_Profiles(aRule.nProfiles);

        HRESULT hr = pFwRules->Add(pFwRule);

        if (S_OK != hr)
        {
            return false;
        }

        return true;
    }

    bool CWAdvancedFirewall::RemoveRule(PCWCHAR szRuleName)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        INetFwRules* pFwRules = NULL;

        if (S_OK != m_pPolicy->get_Rules(&pFwRules))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRules, pFwRules);
        
        _bstr_t bstrName = szRuleName;

        if (S_OK != pFwRules->Remove(bstrName))
        {
            return false;
        }

        return true;
    }

    bool CWAdvancedFirewall::UpdateRule(TWAFRule& aRule)
    {
        if (0 == aRule.strName.length())
        {
            return false;
        }

        if (!m_pPolicy)
        {
            return false;
        }

        INetFwRules* pFwRules = NULL;
        INetFwRule* pFwRule = NULL;

        if (S_OK != m_pPolicy->get_Rules(&pFwRules))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRules, pFwRules);

        _bstr_t bstrName = aRule.strName.c_str();

        HRESULT hr = pFwRules->Item(bstrName, &pFwRule);

        if (S_OK != hr)
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRule, pFwRule);

        pFwRule->put_Enabled(aRule.bEnabled ? VARIANT_TRUE : VARIANT_FALSE);

        switch (aRule.eAction)
        {
        case WAF_ACTION_TYPE_BLOCK:
            pFwRule->put_Action(NET_FW_ACTION_BLOCK);
            break;
        case WAF_ACTION_TYPE_ALLOW:
            pFwRule->put_Action(NET_FW_ACTION_ALLOW);
            break;
        default:
            return false;
        }

        switch (aRule.eDirection)
        {
        case WAF_DIRECTION_TYPE_IN:
            pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
            break;
        case WAF_DIRECTION_TYPE_OUT:
            pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
            break;
        default:
            break;
        }

        _bstr_t bstrDesc = aRule.strDesc.c_str();
        _bstr_t bstrGroup = aRule.strGroup.c_str();
        _bstr_t bstrAppName = aRule.strAppName.c_str();
        _bstr_t bstrLocalAddr = aRule.strLocalAddr.c_str();
        _bstr_t bstrLocalLPorts = aRule.strLocalPorts.c_str();
        _bstr_t bstrRemoteAddr = aRule.strRemoteAddr.c_str();
        _bstr_t bstrRemotePorts = aRule.strRemotePorts.c_str();
        _bstr_t bstrServiceName = aRule.strServiceName.c_str();

        if (bstrDesc.length() > 0)
        {
            pFwRule->put_Description(bstrDesc);
        }

        if (bstrGroup.length() > 0)
        {
            pFwRule->put_Grouping(bstrGroup);
        }

        if (bstrAppName.length() > 0)
        {
            pFwRule->put_ApplicationName(bstrAppName);
        }

        if (bstrServiceName.length() > 0)
        {
            pFwRule->put_ServiceName(bstrServiceName);
        }

        pFwRule->put_Protocol(aRule.nProtocol);

        pFwRule->put_LocalAddresses(bstrLocalAddr);

        pFwRule->put_LocalPorts(bstrLocalLPorts);

        pFwRule->put_RemoteAddresses(bstrRemoteAddr);

        pFwRule->put_RemotePorts(bstrRemotePorts);

        pFwRule->put_Profiles(aRule.nProfiles);

        return true;
    }

    bool CWAdvancedFirewall::GetRulesByName(PCWCHAR szRuleName, TWAFRule& aRule)
    {
        if (!m_pPolicy)
        {
            return false;
        }

        INetFwRules* pFwRules = NULL;
        INetFwRule* pFwRule = NULL;

        if (S_OK != m_pPolicy->get_Rules(&pFwRules))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRules, pFwRules);

        _bstr_t bstrName = szRuleName;

        if (S_OK != pFwRules->Item(bstrName, &pFwRule))
        {
            return false;
        }

        CREATE_AUTO_RELEASE_PTR(INetFwRule, pFwRule);

        GetWAdvancedFirewallRuleInfo(aRule, pFwRule);

        return true;
    }
}