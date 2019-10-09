[TOC]
###Configuration File Description

##### 1. MT4 Connection Configuration
   + host
     - mt4 addr/ mt4 dc addr
   + login
     - mt4 manager account
   + passwd
     - mt4 manager account password  
   + lib
     - mt4 manager api dll path, relative to the server path
</br>
##### 2. Http Server Configuration
   
  **the key can't modify, the value can modify, it means that the uri can be customized.**
   + port
      - http server listen port
   + account-group-common
      - the key sets the uri of Groups's Common Info
   + account-group-permissions
      - the key sets the uri of Group's permissions
   + account-group-archiving
      - the key sets the uri of Group's archive
   + account-group-margins
      - the key sets the uri of Group's margin
   + account-group-securities
      - the key sets the uri of Group's security
   + account-group-symbols
      - the key sets the uri of Group's symbols
   + account-group-reports
      - the key sets the uri of Group's reports
   + common-account-groups
      - the key sets the uri of Groups's name
   + common-symbol-groups
      - the key sets the uri of Securities's name
   + account-group-securities-auto
      - the key sets the uri of Groups's Securities Auto, different with **account-group-securities**
   + account-configuration
      - the key sets the uri of Account's Info
   + datafeed-configuration
      - the key get datafeeder configuraiton
   + common-configuration
      - the key get common configuration
   +  ip-access-configuration
      - the key get access list configuration
   +  symbols-list
      - the key get symbols list
   + symbol-configuration
      - the key get specific symbol   
   + data-center-configuration
      - the key get dc list configuration
   +  plugin-configuration
      - the key get plugin list configuration
   +  performance-configuration
      - the key get performance status  
</br>

###Interface Description
#### Response message Description
  + code
    - 0: ok
    - 1: bad url
    - 2: bad method
    - 3: param invalid
    - 4: server internal error
  + description
    - code explanation
  + response
    - real response message
</br> 
##### 1. Get Groups Common Info
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/common**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/common?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group common success.",
    "response": {
        "Group": "BRD",
        "GroupCommon": {
            "group": "",
            "enable": 0,
            "otp_mode": 0,
            "company": "",
            "support_page": "",
            "default_deposit": 0,
            "currency": "",
            "default_leverage": 0,
            "interestrate": 0
        }
    }
}
```
```
{
    "code": 3,
    "description": "Group is not existed, pls check.",
    "response": {}
}
```

##### 2. Set Groups Common Info
  + Method: POST
  + Uri: **/account/group/common**
  + Params
  ```
  {
        "Group": "BRD",
        "GroupCommon": {
            "group": "",
            "enable": 0,
            "otp_mode": 0,
            "company": "",
            "support_page": "",
            "default_deposit": 0,
            "currency": "",
            "default_leverage": 0,
            "interestrate": 0
        }
    }
  ```
  + Request Examples
```
  http://localhost:9000/account/group/common
```
  + Response
```
{
    "code": 0,
    "description": "update group common success.",
    "response": {}
}
```
```
{
    "code": 3,
    "description": "param invalid.please check and try again.",
    "response": {}
}
```
```
{
    "code": 4,
    "description": "update group common failed.",
    "response": {}
}
```
##### 3. Get Groups Permissions Info
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/permissions**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/permissions?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group permission success.",
    "response": {
        "Group": "BRD",
        "GroupPermission": {
            "timeout": 7,
            "news": 0,
            "news_language": [
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
            ],
            "news_language_total": 0,
            "maxsecurities": 4096,
            "use_swap": 1,
            "rights": 47,
            "check_ie_prices": 1,
            "maxpositions": 0,
            "close_reopen": 0,
            "hedge_prohibited": 0,
            "close_fifo": 0,
            "unused_rights": [
                0,
                0
            ],
            "securities_hash": "\u0018\ufffd\u0016y\ufffdo\t\u0016h\ufffd.<\u0007\ufffd\ufffd\ufffd"
        }
    }
}
```
##### 4. Set Groups Permissions Info
  + Method: POST
  + Uri: **/account/group/permissions**
  + Params
```
{
        "Group": "BRD",
        "GroupPermission": {
            "timeout": 7,
            "news": 0,
            "news_language": [
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
            ],
            "news_language_total": 0,
            "maxsecurities": 4096,
            "use_swap": 1,
            "rights": 47,
            "check_ie_prices": 1,
            "maxpositions": 0,
            "close_reopen": 0,
            "hedge_prohibited": 0,
            "close_fifo": 0,
            "unused_rights": [
                0,
                0
            ],
            "securities_hash": "\u0018\ufffd\u0016y\ufffdo\t\u0016h\ufffd.<\u0007\ufffd\ufffd\ufffd"
        }
    }
```
  + Request Examples
```
  http://localhost:9000/account/group/permissions
```
  + Response Examples
```
{
    "code": 0,
    "description": "update group permission success.",
    "response": {}
}
```
```
{
    "code": 4,
    "description": "update group permission failed.",
    "response": {}
}
```
##### 5. Get Groups Archiving
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/archiving**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/archiving?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group archive success.",
    "response": {
        "Group": "BRD",
        "GroupArchive": {
            "archive_period": 0,
            "archive_max_balance": 0,
            "archive_pending_period": 0
        }
    }
}
```
##### 6. Set Groups Archiving
  + Method: POST
  + Uri: **/account/group/archiving**
  + Params
```
{
        "Group": "BRD",
        "GroupArchive": {
            "archive_period": 0,
            "archive_max_balance": 0,
            "archive_pending_period": 0
        }
    }
```
  + Request Examples
```
  http://localhost:9000/account/group/permissions
```
  + Response Examples
```
{
    "code": 0,
    "description": "update group archive success.",
    "response": {}
}
```
##### 7. Get Groups Margins
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/margins**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/margins?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group margin success.",
    "response": {
        "Group": "BRD",
        "GroupMargin": {
            "margin_call": 150,
            "margin_stopout": 50,
            "margin_type": 0,
            "margin_mode": 1,
            "credit": 0,
            "stopout_skip_hedged": 0,
            "hedge_largeleg": 1
        }
    }
}
```

##### 8. Set Groups Margins
  + Method: POST
  + Uri: **/account/group/margins**
  + Params
  ```
  {
        "Group": "BRD",
        "GroupMargin": {
            "margin_call": 150,
            "margin_stopout": 50,
            "margin_type": 0,
            "margin_mode": 1,
            "credit": 0,
            "stopout_skip_hedged": 0,
            "hedge_largeleg": 1
        }
    }
  ```
  + Request Examples
```
  http://localhost:9000/account/group/margins
```
  + Response Examples
```
{
    "code": 0,
    "description": "update group margin success.",
    "response": {}
}
```

##### 9. Get Groups Securities
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/securities**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/securities?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group securities success.",
    "response": {
        "Group": "BRD",
        "GroupSec": [
            {
                "index": 0,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 5000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            },
            {
                "index": 1,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 5000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            },
            {
                "index": 2,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 2000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            }
        ]
    }
}
```
##### 10. Set Groups Securities
  + Method: POST
  + Uri: **/account/group/securities**
  + Params
  ```
  {
        "Group": "BRD",
        "GroupSec": [
            {
                "index": 0,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 5000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            },
            {
                "index": 1,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 5000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            },
            {
                "index": 2,
                "show": 1,
                "trade": 1,
                "execution": 1,
                "comm_base": 0,
                "comm_type": 1,
                "comm_lots": 0,
                "comm_agent": 0,
                "comm_agent_type": 1,
                "spread_diff": 0,
                "lot_min": 1,
                "lot_max": 2000,
                "lot_step": 1,
                "ie_deviation": 2,
                "confirmation": 0,
                "trade_rights": 0,
                "ie_quick_mode": 0,
                "autocloseout_mode": 0,
                "comm_tax": 0,
                "comm_agent_lots": 0,
                "freemargin_mode": 1
            }
        ]
    }
  ```
  + Request Examples
```
  http://localhost:9000/account/group/securities
```
  + Response Examples
```
{
    "code": 0,
    "description": "update group securities success.",
    "response": {}
}
```

##### 11. Get Groups Symbols
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/symbols**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/symbols?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group symbols success.",
    "response": {
        "Group": "BRD",
        "GroupSymbol": []
    }
}
```

##### 12. Set Groups Symbols
  + Method: POST
  + Uri: **/account/group/symbols**
  + Params
  ```
 {
        "Group": "BRD",
        "GroupSymbol": [
            {
                "symbol":"AUDCAD",
                "swap_short":1000.0,
                "swap_long":1000.0,
                "margin_divider":10.0
            }
        ]
    }
  ```

  + Request Examples
```
  http://localhost:9000/account/group/symbols
```

  + Response Examples
```
{
    "code": 0,
    "description": "update group securities success.",
    "response": {}
}
```

##### 13. Get Groups Reports
  + Method: GET
  + Uri: read from the configuration file, default is **/account/group/reports**
  + Params: Group=XXX
  + Request Examples
```
  http://localhost:9000/account/group/reports?Group=BRD
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group report success.",
    "response": {
        "Group": "BRD",
        "GroupReport": {
            "reports": 1,
            "copies": 0,
            "smtp_server": "",
            "template_path": "",
            "smtp_login": "",
            "smtp_passwd": "",
            "support_email": "",
            "signature": ""
        }
    }
}
```

##### 14. Set Groups Reports
  + Method: POST
  + Uri: **/account/group/reports**
  + Params
  ```
 {
        "Group": "BRD",
        "GroupReport": {
            "reports": 1,
            "copies": 0,
            "smtp_server": "",
            "template_path": "",
            "smtp_login": "",
            "smtp_passwd": "",
            "support_email": "",
            "signature": ""
        }
    }
  ```
  + Request Examples
```
  http://localhost:9000/account/group/reports
```
  + Response Examples
```
{
    "code": 0,
    "description": "update group report success.",
    "response": {}
}
```

##### 15. Get Groups List
  + Method: GET
  + Uri: read from the configuration file, default is **/account/common/account/groups**
  + Params: None
  + Request Examples
```
  http://localhost:9000/account/common/account/groups
```
  + Response Examples
```
{
    "code": 0,
    "description": "get group names success.",
    "response": {
        "total": 43,
        "groups": [
            "manager",
            "system",
            "TBRD-M00A",
            "TBRD-M00AC",
            "TBRD-M00AC-Tech",
            "BRD",
            "STP",
            "STP_TW",
            "preliminary",
            "TBRD-M00B",
            "STP_NO_200",
            "BRD_NO_200",
            "TBRD_NO_200",
            "STP_100_GBP",
            "BRD_100_GBP",
            "TBRD_100_GBP",
            "TBRD_WL_100_GBP",
            "Archive",
            "defAddSpread",
            "defTarSpread",
            "defMinSpread",
            "defMaxSpread",
            "Coverage",
            "STP_UK_GBP",
            "BRD_UK_GBP",
            "TBRD_UK_GBP",
            "TBRD_WL_UK_GBP",
            "BRD_RM_UK_GBP",
            "TEST_ABOOK",
            "demo-forSync",
            "TESE_TECH",
            "TBRD_IG_Tech",
            "TBRD_IG",
            "BRD_IG",
            "STP_IG",
            "TBRD_GB",
            "BRD_GB",
            "STP_GB",
            "TBRD_ISLAMIC",
            "BRD_ISLAMIC",
            "STP_ISLAMIC",
            "TBRD_GB_Tech",
            "TBRD_ISLMC_Tech"
        ]
    }
}
```

##### 16. Get Securities List
  + Method: GET
  + Uri: read from the configuration file, default is **/common/symbol/groups**
  + Params: None
  + Request Examples
```
  http://localhost:9000/common/symbol/groups
```
  + Response Examples
```
{
    "code": 0,
    "description": "get sucurities name success.",
    "response": {
        "total": 4,
        "securities": [
            {
                "name": "Gold-GB-1",
                "description": "Gold Global 100 Leverage"
            },
            {
                "name": "Silver-GB-1",
                "description": "Silver Global 100 Leverage"
            },
            {
                "name": "Oil-GB-1",
                "description": "Oil Global 100 Leverage"
            },
            {
                "name": "Gold-GB-H",
                "description": "Gold Global 400 Leverage"
            }
        ]
    }
}
```

##### 17. Set Account Info(Modify Account Password for now)
  + Method: POST
  + Uri: read from the configuration file, default is **/account/configuration**
  + Params:
  ```
  {
      "login":"123456",
      "password":"qwe123",
      "enable_change_password":0
  }
  ```
  + Request Examples
```
  http://localhost:9000/account/configuration
```
  + Response Examples
```
{
    "code":0,
    "description":"update account configuration success."
    "response":{}
}
```

##### 18. Get DataFeeder Info
  + Method: Get
  + Uri: read from the configuration file, default is **/global/datafeeder/conf
  + Params: none
  + Request example
  ```
  http://localhost:9000/global/datafeeder/conf
  ```
  + Response example
  ```
  {
    "code": 0,
    "description": "get datafeeder list success.",
    "response": {
        "datafeed": [
            {
                "name": "MetaTrader 4 Feeder - DC_Internal",
                "file": "mt4feeder.feed",
                "server": "dcm.tigerwit.com:443",
                "login": "70002",
                "pass": "5211314tg",
                "keywords": "",
                "enable": 1,
                "mode": 0,
                "timeout": 10,
                "timeout_reconnect": 5,
                "timeout_sleep": 5,
                "attemps_sleep": 5,
                "news_langid": 0
            },
            {
                "name": "CFHServerFeeder",
                "file": "CFHServerFeeder.feed",
                "server": "127.0.0.1:30000",
                "login": "cfh",
                "pass": "Password1",
                "keywords": "",
                "enable": 1,
                "mode": 0,
                "timeout": 120,
                "timeout_reconnect": 5,
                "timeout_sleep": 60,
                "attemps_sleep": 5,
                "news_langid": 0
            }
        ]
    }
}
  ```

##### 19. Get Common Info
+ Method: Get
  + Uri: read from the configuration file, default is **/global/common/conf
  + Params: none
  + Request example
  ```
  http://localhost:9000/global/common/conf
  ```
  + Response example
  ```
  {
    "code": 0,
    "description": "get global common success.",
    "response": {
        "owner": "Tiger Wit Group Limited (4.00 build 1220, 13 Sep 2019), activated, expiry date: 2019.12.16",
        "name": "Local Tensor",
        "address": 0,
        "port": 443,
        "timeout": 180,
        "enable_demo": 1,
        "timeofdemo": 14,
        "daylightcorrection": 0,
        "timezone_real": 3,
        "timezone": 3,
        "timesync": "pool.ntp.org",
        "minclient": 400,
        "minapi": 1,
        "feeder_timeout": 30,
        "keepemails": 3,
        "endhour": 23,
        "endminute": 59,
        "optimization_time": 237,
        "optimization_lasttime": 1569297434,
        "optimization_counter": 0,
        "antiflood": 1,
        "floodcontrol": 150,
        "liveupdate_mode": 1,
        "lastorder": 31754836,
        "lastlogin": 1265866054,
        "lostlogin": 1265866053,
        "rollovers_mode": 0,
        "path_database": "D:\\MetaTrader4Server\\bases",
        "path_history": "D:\\MetaTrader4Server\\history",
        "path_log": "D:\\MetaTrader4Server\\logs",
        "overnight_last_day": 23,
        "overnight_last_time": 1569283199,
        "overnight_prev_time": 1569196799,
        "overmonth_last_month": 7,
        "adapters": "Red Hat VirtIO Ethernet Adapter|",
        "bind_adresses": [
            2689269417,
            1192759724,
            0,
            0,
            0,
            0,
            0,
            0
        ],
        "server_version": 400,
        "server_build": 1220,
        "web_adresses": [
            16777343,
            4294967295,
            4294967295,
            4294967295,
            4294967295,
            4294967295,
            4294967295,
            4294967295
        ],
        "statement_mode": 0,
        "monthly_state_mode": 0,
        "keepticks": 15,
        "statement_weekend": 0,
        "last_activate": 0,
        "stop_last": 1569217376,
        "stop_delay": 300,
        "stop_reason": 0,
        "account_url": ""
    }
  }
  ```

##### 20. Get Access List Info
  + Method: Get
  + Uri: read from the configuration file, default is **/global/ip-access/conf
  + Params: none
  + Request example
  ```
  http://localhost:9000/global/ip-access/conf
  ```
  + Response example
  ```
  {
    "code": 0,
    "description": "get access list success.",
    "response": {
        "Access-List": [
            {
                "action": 2,
                "from": 805208550,
                "to": 805208550,
                "comment": "MT4 Trial"
            },
            {
                "action": 2,
                "from": 2147558918,
                "to": 2147558918,
                "comment": "MT4 Dev New in SG"
            }
        ]
    }
  }
  ```

##### 21. Get Symbols List Info
+ Method:Get
+ Uri: read from the configuration file, default is **//global/symbols/list
+ Params: none
+ Request example
```
http://localhost:9000/global/symbols/list
```
+ Response example
```
{
    "code": 0,
    "description": "get symbol list succes.",
    "response": {
        "size": 2,
        "symbols-list": [
            "100GBP",
            "100GBP20"
        ]
    }
}
```

##### 22. Get Specific Symbol Info
+ Method:POST
+ Uri: read from the configuration file, default is **/global/symbol/conf
+ Params:
```
{"symbol":"AUDCAD"}
```
+ Request example
```
http://localhost:9000/global/symbol/conf
```
+ Response example
```
{
    "code": 0,
    "description": "get symbol conf success.",
    "response": {
        "symbol": "AUDCAD",
        "description": "Australian Dollar vs Canadian Dollar",
        "source": "",
        "currency": "AUD",
        "type": 4,
        "digits": 5,
        "trade": 2,
        "background_color": 14804223,
        "count": 6,
        "count_original": 0,
        "realtime": 1,
        "starting": 0,
        "expiration": 0,
        "sessions": [
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            }
        ],
        "profit_mode": 0,
        "filter": 0,
        "filter_counter": 3,
        "filter_limit": 0.1,
        "filter_smoothing": 0,
        "logging": 1,
        "spread": 0,
        "spread_balance": 0,
        "exemode": 2,
        "swap_enable": 1,
        "swap_type": 0,
        "swap_long": -3.86,
        "swap_short": -3.86,
        "swap_rollover3days": 3,
        "contract_size": 100000.0,
        "tick_value": 0.0,
        "tick_size": 0.0,
        "stops_level": 100,
        "gtc_pendings": 1,
        "margin_mode": 0,
        "margin_initial": 0.0,
        "margin_maintenance": 0.0,
        "margin_hedged": 100000.0,
        "margin_divider": 1.0,
        "point": 0.00001,
        "multiply": 100000.0,
        "bid_tickvalue": 0.0,
        "ask_tickvalue": 0.0,
        "long_only": 0,
        "instant_max_volume": 0,
        "margin_currency": "AUD",
        "freeze_level": 0,
        "margin_hedged_strong": 0,
        "value_date": 0,
        "quotes_delay": 0,
        "swap_openprice": 0,
        "swap_variation_margin": 0
    }
}
```

##### 23. Get DC List Info
+ Method:GET
+ Uri: read from the configuration file, default is **/global/dc/conf
+ Params: none
+ Request example
```
http://localhost:9000/global/dc/conf
```
+ Response example
```
{
    "code": 0,
    "description": "get dc list success.",
    "response": {
        "DC-List": [
            {
                "server": "128.1.40.103",
                "ip": 1730675072,
                "description": "DC-SG",
                "isproxy": 0,
                "priority": 0,
                "loading": 4294967295,
                "ip_internal": 83994816
            },
            {
                "server": "128.1.40.53",
                "ip": 891814272,
                "description": "HA-SG",
                "isproxy": 1,
                "priority": 255,
                "loading": 4294967295,
                "ip_internal": 1342286016
            },
            {
                "server": "127.0.0.1",
                "ip": 16777343,
                "description": "localhost",
                "isproxy": 0,
                "priority": 0,
                "loading": 4294967295,
                "ip_internal": 16777343
            }
        ]
    }
}
```

##### 24. Get Plugin List Info
+ Method:GET
+ Uri: read from the configuration file, default is **/global/plugin-list/conf
+ Params: none
+ Request example
```
http://localhost:9000/global/plugin-list/conf
```
+ Response example
```
{
    "code": 0,
    "description": "get plugin list success.",
    "response": {
        "Plugin-List": [
            {
                "plugin": {
                    "file": "ReasonModify.dll",
                    "info": {
                        "name": "ReasonModify",
                        "version": 101,
                        "copyright": "TigerWit Corp."
                    },
                    "enabled": 1,
                    "configurable": 1,
                    "manager_access": 0
                },
                "params-total": 0,
                "params": []
            },
            {
                "plugin": {
                    "file": "CFH FIX API.dll",
                    "info": {
                        "name": "CFH_FIX_API",
                        "version": 272,
                        "copyright": "2017  (2, 72, 0, 46)"
                    },
                    "enabled": 1,
                    "configurable": 1,
                    "manager_access": 0
                },
                "params-total": 3,
                "params": [
                    {
                        "name": "Auto Trade orders below min",
                        "value": "No"
                    },
                    {
                        "name": "Bridge Groups",
                        "value": ",STP,STP*,*STP,BRD*,*BRD,TRUE-MONEY_CFH,manager,TBRD-M00A,TBRD_GB,TBRD_ISLAMIC,TBRD_IG,"
                    },
                    {
                        "name": "Bridge ID",
                        "value": ""
                    }
                ]
            },
            {
                "plugin": {
                    "file": "Quote_Mantou.dll",
                    "info": {
                        "name": "Quote_Mantou",
                        "version": 101,
                        "copyright": "TigerWit Corp."
                    },
                    "enabled": 0,
                    "configurable": 0,
                    "manager_access": 0
                },
                "params-total": 0,
                "params": []
            },
            {
                "plugin": {
                    "file": "loganalyser.dll",
                    "info": {
                        "name": "LogAnalyser Settings",
                        "version": 101,
                        "copyright": "MetaQuotes Software Corp."
                    },
                    "enabled": 1,
                    "configurable": 1,
                    "manager_access": 0
                },
                "params-total": 2,
                "params": [
                    {
                        "name": "Dangerous Avg Trade Time",
                        "value": "300"
                    },
                    {
                        "name": "Dangerous Confirm Deviation",
                        "value": "4"
                    }
                ]
            },
            {
                "plugin": {
                    "file": "toptraders.dll",
                    "info": {
                        "name": "TopTraders Settings",
                        "version": 100,
                        "copyright": "MetaQuotes Software Corp."
                    },
                    "enabled": 1,
                    "configurable": 1,
                    "manager_access": 0
                },
                "params-total": 6,
                "params": [
                    {
                        "name": "Groups",
                        "value": ",*,"
                    },
                    {
                        "name": "Maximum Loss Trades",
                        "value": "10"
                    },
                    {
                        "name": "Maximum Profit Trades",
                        "value": "10"
                    },
                    {
                        "name": "Maximum Users",
                        "value": "25"
                    },
                    {
                        "name": "Minimum Profit Factor",
                        "value": "1.0"
                    },
                    {
                        "name": "Minimum Trades",
                        "value": "10"
                    }
                ]
            },
            {
                "plugin": {
                    "file": "TradeCollector.dll",
                    "info": {
                        "name": "Trade Collector",
                        "version": 102,
                        "copyright": "TigerWit Corp."
                    },
                    "enabled": 1,
                    "configurable": 1,
                    "manager_access": 0
                },
                "params-total": 5,
                "params": [
                    {
                        "name": "Coverage",
                        "value": "7001"
                    },
                    {
                        "name": "Enable",
                        "value": "YES"
                    },
                    {
                        "name": "Host",
                        "value": "127.0.0.1"
                    },
                    {
                        "name": "Port",
                        "value": "9001"
                    },
                    {
                        "name": "Symbol",
                        "value": "AUDCAD;AUDCAD200;AUDCAD400"
                    }
                ]
            }
        ]
    }
}
```

##### 25. Get Server Performance Info
+ Method:POST
+ Uri: read from the configuration file, default is **/global/performance/conf
+ Params: the timestamp is (Standard Timestamp + 3\*60\*60),the performance list is 60s interval.
```
{"from":1569328888}
```
+ Request example
```
http://localhost:9000/global/performance/conf
```
+ Response example
```
{
    "code": 0,
    "description": "get performance conf success.",
    "response": {
        "Performance": [
            {
                "ctm": 1569328920,
                "users": 3,
                "cpu": 58,
                "freemem": 953620,
                "network": 24,
                "sockets": 77
            },
            {
                "ctm": 1569328980,
                "users": 3,
                "cpu": 9,
                "freemem": 1125052,
                "network": 36,
                "sockets": 66
            }
        ]
    }
}
```

##### 26. Get Server Holiday Info
+ Method:POST
+ Uri: read from the configuration file, default is /get/holiday
+ Params: None

+ Request example
```
http://localhost:9000/get/holiday
```
+ Response example
```
{
    "code": 0,
    "description": "get holiday conf success.",
    "response": {
        "Holiday": [
            {
                "year": 2014,
                "month": 12,
                "day": 24,
                "from": 0,
                "to": 1230,
                "symbol": "Forex",
                "description": "Christmas Day",
                "enable": 1
            },
            {
                "year": 2014,
                "month": 12,
                "day": 24,
                "from": 0,
                "to": 1230,
                "symbol": "Forex-2",
                "description": "Christmas Day",
                "enable": 1
            },
            {
                "year": 2014,
                "month": 12,
                "day": 24,
                "from": 0,
                "to": 1230,
                "symbol": "XAUUSD",
                "description": "Christmas Day",
                "enable": 1
            }
        ]
    }
}
```

##### 27. Update Symbol Session Info
+ Method:POST
+ Uri: read from the configuration file, default is /set/symbol/session
+ Params: 
```
{
"symbol":"AUDCAD400",
"sessions": [
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 5,
                        "close_hour": 23,
                        "close_min": 55
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            },
            {
                "quote_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ],
                "trade_session": [
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    },
                    {
                        "open_hour": 0,
                        "open_min": 0,
                        "close_hour": 0,
                        "close_min": 0
                    }
                ]
            }
        ]
}
```

+ Request example
```
http://localhost:9000/get/holiday
```
+ Response example
```
{
    "code": 0,
    "description": "update symbol sessions success.",
    "response": {}
}
```