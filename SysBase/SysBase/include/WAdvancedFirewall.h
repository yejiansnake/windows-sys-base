#include "SysBase.h"
#include <Icftypes.h>
#include <Netfw.h>

namespace SysBase
{
    //出入站类型
    enum EWAFDirectionType
    {
        WAF_DIRECTION_TYPE_IN = 1,      //入站
        WAF_DIRECTION_TYPE_OUT = 2,     //出站

    };

    //规则活动类型
    enum EWAFActionType
    {
        WAF_ACTION_TYPE_BLOCK = 0,  //阻止连接
        WAF_ACTION_TYPE_ALLOW,      //允许连接
    };

    //配置类型
    enum EWAFProfileType
    {
        WAF_PROFILE_TYPE_DOMAIN   = 1,              //域
        WAF_PROFILE_TYPE_PRIVATE  = 2,              //专用
        WAF_PROFILE_TYPE_PUBLIC   = 4,              //共用
        WAF_PROFILE_TYPE_ALL      = 0x7FFFFFFF,     //全部
    };

    struct TWAFRule
    {
        TWAFRule()
        {
            eAction = WAF_ACTION_TYPE_BLOCK;
            eDirection = WAF_DIRECTION_TYPE_IN;
            bEnabled = false;
            nProfiles = 0;
            nProtocol = WAF_PROFILE_TYPE_ALL;
        }

        wstring strName;                //规则名称（可重复）
        EWAFDirectionType eDirection;   //出入站(EDirectionType)
        bool    bEnabled;               //启用或禁用（0:禁用;1:启用）
        EWAFActionType eAction;         //规则活动类型（EActionType）
        wstring strAppName;             //应用该规则的程序全路径
        wstring strServiceName;         //应用该规则的服务名
        wstring strDesc;                //描述
        wstring strGroup;               //所属的组
        INT32   nProtocol;              //协议（6:TCP; 17:UDP）[0:任何,HOPOPT;1:ICMPv4;2:IGMP;6:TCP;17:UDP;41:IPv6;43:IPv6-Route;44:IPv6-Frag;47:GRE;58:ICMPv6;59:IPv6-NoNxt;60:IPv6-0pts;112:VRRP;113:PGM;115:L2TP]
        wstring strLocalAddr;           //本地地址(全部填""或"*")
        wstring strLocalPorts;          //本地端口(全部填""或"*")
        wstring strRemoteAddr;          //本地地址(全部填""或"*")
        wstring strRemotePorts;         //本地端口(全部填""或"*")
        INT32   nProfiles;              //所属配置规则（EProfileType 中的一个或多个）
    };

    //WINDOWS高级防火墙
    class CWAdvancedFirewall
    {
    public:

        CWAdvancedFirewall();

        virtual ~CWAdvancedFirewall();

        bool GetStatus(EWAFProfileType type, bool& bEnabled);

        bool SetStatus(EWAFProfileType type, bool bEnable);

        bool GetRuleAll(list<TWAFRule>& ruleList);

        bool AddRule(TWAFRule& aRule);

        bool RemoveRule(PCWCHAR szRuleName);

        bool UpdateRule(TWAFRule& aRule);

        bool GetRulesByName(PCWCHAR szRuleName, TWAFRule& aRule);

    protected:
    private:

        INetFwPolicy2* m_pPolicy;
    };
}