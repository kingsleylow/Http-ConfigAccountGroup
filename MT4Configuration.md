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
